/*
 * Word count application with one process per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "word_count.h"
#include "word_helpers.h"
/*
    fork seperate child process for each file. Merge output of each process into final output
    each child process sends output to parent via pipe and parent uses merge_counts to merge results

*/



/*
 * Read stream of counts and accumulate globally.
 * More specifically, reads from fprint stream and can merge the counts together
 */
void merge_counts(word_count_list_t *wclist, FILE *count_stream) {
    char *word;
    int count;
    int rv;
    while ((rv = fscanf(count_stream, "%8d\t%ms\n", &count, &word)) == 2)
    {
        add_word_with_count(wclist, word, count);\
    }
    if ((rv == EOF) && (feof(count_stream) == 0)) {
        perror("could not read counts");
    } else if (rv != EOF) {
        fprintf(stderr, "read ill-formed count (matched %d)\n", rv);
    }
}

/*
 * main - handle command line, spawning one process per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts);

    if (argc <= 1) {
        /* Process stdin in a single process. */
        count_words(&word_counts, stdin);
    } else {
        /* Process each file in a separate process. */
        int i;
        //use multiple pipes because one pipe is not enough
        int pipefds[argc - 1][2];
        
        /* Process each file in a separate process. */
        for (i = 1; i < argc; i++) {
            if (pipe(pipefds[i-1]) == -1) {
                perror("pipe");
                exit(1);
            }
            pid_t pid = fork();
            if (pid == 0) {
                /* Child process. */
                close(pipefds[i-1][0]); 
                
                FILE *infile = fopen(argv[i], "r");
                if (infile == NULL) {
                    perror("fopen");
                    exit(1);
                }
                printf("child process %d started\n", i);
                count_words(&word_counts, infile);
                fclose(infile);

                FILE *pipe_out = fdopen(pipefds[i-1][1], "w");
                if (pipe_out == NULL) {
                    perror("fdopen");
                    fclose(infile); 
                    exit(1);
                }

                fprint_words(&word_counts, pipe_out);
                fflush(pipe_out); 
                fclose(pipe_out);
                close(pipefds[i-1][1]); 
                exit(0);
            } else {
                close(pipefds[i-1][1]); 
            }
        }
        
        
        
        for (i = 1; i < argc; i++) {
            FILE *pipe_stream = fdopen(pipefds[i-1][0], "r");
            if (!pipe_stream) {
                perror("fdopen");
                continue;
            } 
            merge_counts(&word_counts, pipe_stream);
            fclose(pipe_stream); // Don't forget to close the stream
        }

        //clean up to avoid zombie processes
        for (i = 1; i < argc; i++) {
            wait(NULL); 
        }
    }


    /* Output final result of all process' work. */
    printf("len_words: %d\n", len_words(&word_counts));
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}
