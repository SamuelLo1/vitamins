/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright (C) 2019 University of California, Berkeley
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
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "word_count.h"
#include "word_helpers.h"

//struct to hold arguments for each count_words call by create threads

word_count_list_t word_counts;

//wrapper function for count_words to be used with pthread_create
void *count_words_wrapper(void *filename) {
    //extract arguments from the struct
    FILE *infile = fopen(filename, "r");
    //check if file opened successfully
    if (infile == NULL) {
        perror("fopen");
        return NULL;
    }
    printf("before count_word\n");
    count_words(&word_counts, infile);
    printf("after count_words\n");
    //handle freeing of owned resources of targs not on stack
    fclose(infile);
    pthread_exit(NULL);
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    /* Don't forget that this word_counts_t works different because its a struct
       with lock and lst instead of regular PINTOS list. 
    */
    printf("before init_words\n");
    init_words(&word_counts);
    if (argc <= 1) {
        /* Process stdin in a single thread. */
        count_words(&word_counts, stdin);
    } else {
        //initialize threads
        pthread_t threads[argc-1];
        //go through each file
        int i;
        for (i = 1; i < argc; i++) {
            if (pthread_create(&threads[i-1], NULL, count_words_wrapper, (void *)argv[i])){
                perror("pthread_create did not succeed");
                exit(1);
            }
        }
        //join the threads to continue executing main
        for (i = 0; i < argc-1; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    /* Output final result of all threads' work. */
    printf("%d\n", len_words(&word_counts));
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}
