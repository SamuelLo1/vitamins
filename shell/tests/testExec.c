#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Print the arguments passed to the program
    printf("testExec initiated:\n");
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d]: %s\n", i, argv[i]);
    }
    // Mimic a process by sleeping for 5 seconds
    printf("Sleeping for 2 seconds...\n");
    sleep(2);
    printf("Done sleeping. Exiting now.\n");
    return 0;
}