#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Print the arguments passed to the program
    printf("testExec initiated:\n");
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d]: %s\n", i, argv[i]);
    }
    // wait for interrupt signal to exit
    printf("Waiting for interrupt signal to exit...\n");
    sleep(5);
    return 0;
}