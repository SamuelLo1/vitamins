#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <termios.h> // For tcsetpgrp and tcgetpgrp
#include <sys/wait.h> // For waitpid

volatile sig_atomic_t child_exited = 0;
pid_t child_pid_global; // Global variable to store child's PID

void signal_handler_child(int sig) {
    printf("Child process %d received signal %s\n", getpid(), strsignal(sig));
    exit(0);
}

void signal_handler_parent(int sig) {
    if (sig == SIGCHLD) {
        child_exited = 1;
    }
}

int main() {
    pid_t parent_pid = getpid();

    printf("Parent PID: %d, PGID: %d\n", parent_pid, getpgrp());

    // Ignore signals that might cause the parent to stop interacting with the terminal
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, signal_handler_parent); // Handle SIGCHLD to know when the child exits

    // Set the parent as the initial foreground process group
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
        perror("tcsetpgrp (parent)");
        exit(EXIT_FAILURE);
    }
    pid_t fg_pgid_parent = tcgetpgrp(STDIN_FILENO);
    printf("Parent set as foreground PGID: %d\n", fg_pgid_parent);

    child_pid_global = fork();

    if (child_pid_global == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid_global == 0) {
        // Child process
        setpgid(0, 0); // Create a new process group for the child
        printf("Child PID: %d, PGID: %d\n", getpid(), getpgrp());

        // Child handles these signals to exit
        signal(SIGINT, signal_handler_child);
        signal(SIGTSTP, signal_handler_child);
        signal(SIGQUIT, signal_handler_child);

        // Let the parent set things up
        sleep(1);
        pid_t fg_pgid_child_init = tcgetpgrp(STDIN_FILENO);
        printf("Child in background (initially). Foreground PGID: %d\n", fg_pgid_child_init);

        printf("Child process running in the foreground. Press Ctrl+C to terminate.\n");
        while (1) {
            sleep(1); // Child does its work here
        }
        exit(EXIT_SUCCESS); // Should not reach here under normal signal handling
    } else {
        // Parent process
        printf("Parent created child with PID: %d\n", child_pid_global);
        sleep(1); // Give child a chance to start

        printf("Press Enter to make the child the foreground process group...\n");
        getchar(); // Wait for Enter

        if (tcsetpgrp(STDIN_FILENO, getpgid(child_pid_global)) == -1) {
            perror("tcsetpgrp (set child)");
            exit(EXIT_FAILURE);
        }
        pid_t fg_pgid_after_child = tcgetpgrp(STDIN_FILENO);
        printf("Parent set child's PGID (%d) as foreground. Foreground PGID: %d\n", getpgid(child_pid_global), fg_pgid_after_child);

        printf("Parent is now in the background, waiting for the child to terminate (e.g., via Ctrl+C)...\n");

        // Parent waits for the child to exit
        while (!child_exited) {
            sleep(0.1);
        }

        printf("\nChild process terminated. Parent regaining control of the terminal.\n");

        // Once the child exits, the parent sets itself back as the foreground process group
        if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
            perror("tcsetpgrp (set parent back)");
            exit(EXIT_FAILURE);
        }
        pid_t fg_pgid_after_parent = tcgetpgrp(STDIN_FILENO);
        printf("Parent set as foreground PGID again: %d\n", fg_pgid_after_parent);

        printf("Press Enter to continue parent process...\n");
        signal(SIGINT, SIG_DFL); // Reset signal handling to default
        while(1){
            sleep(0.1);
        }
        printf("Parent process continuing.\n");
    }

    return 0;
}