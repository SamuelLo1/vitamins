#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function
 * parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

/* shell specific commands */
int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

/* takes in fun, cmd name, description per new command introduced */
fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_cd, "cd", "change directory"},
    {cmd_pwd, "pwd", "print working directory"},
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    }
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
    exit(0);
}

/* Gets current working directory */
int cmd_pwd(unused struct tokens *tokens) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd() error");
    }
    return 1;
}

/* Changes the current working directory */
/*
    Tokens are useful. Essentially arguments turned to words
    by the tokenizer that are parsed 
    from the command line. Think of argv in main. 
*/
int cmd_cd(struct tokens *tokens) {
    // can only take in one argument
    if (tokens_get_length(tokens) != 2) {
        fprintf(stderr, "cd: wrong number of arguments\n");
        return 1;
    }
    // get the directory argument
    if (chdir(tokens_get_token(tokens, 1)) != 0) {
        perror("cd");
    }
    return 1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char *cmd) {
    if (cmd != NULL) {
        for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
            if (strcmp(cmd_table[i].cmd, cmd) == 0) {
                return i;
            }
        }
    }
    return -1;
}


void handle_sigtstp(int sig) {
    printf("Child received SIGTSTP\n");
    exit(0); // Optional: exit for debugging
}

/* function to handle forking processes for programs executed via shell */
/*
    Want to ensure each process is in its own process group 
    and that the process is in foreground
    shell should be in the background? 
*/
int execute_prog(char* program, char* args[], char* infile, char* outfile) {
    pid_t main_pid = getpid();
    
    if (program == NULL) {
        fprintf(stderr, "No program specified\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // set the process group id to the child process and move to foreground
        setpgid(getpid(), getpid()); 
        tcsetpgrp(STDIN_FILENO, getpid()); 

        //child process has default interrupt signal handling 
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, handle_sigtstp);
        signal(SIGCONT, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        if (infile != NULL) {
            int in_fd = open(infile, O_RDONLY);
            if (in_fd < 0) {
                perror("open (infile)");
                exit(EXIT_FAILURE);
            }
        
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        // Handle output redirection
        if (outfile != NULL) {
            int out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("open (outfile)");
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        // Same path resolution logic from before (see previous message)
        if (strchr(program, '/')) {
            execv(program, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        // PATH resolution
        char* path_env = getenv("PATH");
        if (path_env == NULL) {
            fprintf(stderr, "PATH not found\n");
            exit(EXIT_FAILURE);
        }

        char* path_copy = strdup(path_env);
        char* dir = strtok(path_copy, ":");
        while (dir != NULL) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, program);
            execv(full_path, args);
            dir = strtok(NULL, ":");
        }

        fprintf(stderr, "%s: command not found\n", program);
        exit(EXIT_FAILURE);
    } else {
        // Parent process (shell)
        setpgid(pid, pid); // Make sure child is in its own group
        tcsetpgrp(STDIN_FILENO, pid); // Give terminal to child
        int status;
        waitpid(pid, &status, 0);
        /*
        restore the shell to accept signals and in foreground if no 
        child processes running
        */
        if (tcsetpgrp(STDIN_FILENO, main_pid) == -1) {
            perror("tcsetpgrp (parent)");
        }
        /* restore default interrupt handling in main process */
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        return WEXITSTATUS(status);
    }
}


/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* If the shell is not currently in the foreground, we must pause the
         * shell until it becomes a foreground process. We use SIGTTIN to pause
         * the shell. When the shell gets moved to the foreground, we'll receive
         * a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later.
         */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

int main(unused int argc, unused char *argv[]) {
    init_shell();
    pid_t shell_pid = getpid();
    printf("Shell PID: %d, PGID: %d\n", shell_pid, shell_pgid);
    
    static char line[4096];
    int line_num = 0;

    /* Only print shell prompts when standard input is not a tty */
    if (shell_is_interactive) {
        fprintf(stdout, "%d: ", line_num);
    }

    while (fgets(line, 4096, stdin)) {
        /* Split our line into words. */
        struct tokens *tokens = tokenize(line);

        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        if (fundex >= 0) {
            cmd_table[fundex].fun(tokens);
        } else {
            /*
            main process ignores interrupt signals, child should handle
            */
            signal(SIGINT, SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            signal(SIGTSTP, SIG_IGN);
            signal(SIGCONT, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            signal(SIGCHLD, SIG_IGN);
            /*  
                parse the cmdline input and attempt to execute the program via fork process
            */
            char *program = NULL;
            char *args[4096];
            char *infile = NULL;
            char *outfile = NULL;
            int arg_index = 0;

            for (size_t i = 0; i < tokens_get_length(tokens); i++) {
                char *tok = tokens_get_token(tokens, i);

                if (strcmp(tok, "<") == 0) {
                    i++;
                    infile = tokens_get_token(tokens, i);
                } else if (strcmp(tok, ">") == 0) {
                    i++;
                    outfile = tokens_get_token(tokens, i);
                } else {
                    args[arg_index++] = tok;
                }
            }
            args[arg_index] = NULL;
            if (arg_index == 0) continue;
            program = args[0];
            execute_prog(program, args, infile, outfile);   
        }

        if (shell_is_interactive) {
            /* Only print shell prompts when standard input is not a tty. */
            fprintf(stdout, "%d: ", ++line_num);
        }

        /* Clean up memory. */
        tokens_destroy(tokens);
    }

    return 0;
}
