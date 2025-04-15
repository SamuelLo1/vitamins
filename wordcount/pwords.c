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
 
 // Struct to hold arguments for each thread
 typedef struct {
     word_count_list_t *word_counts; 
     char *filename;                
 } thread_args_t;
 
 // Wrapper function for count_words to be used with pthread_create
 void *count_words_wrapper(void *args) {
     // Extract arguments from the struct
     thread_args_t *targs = (thread_args_t *)args;
     FILE *infile = fopen(targs->filename, "r");
 
     // Check if file opened successfully
     if (infile == NULL) {
         perror("fopen");
         return NULL;
     }
     count_words(targs->word_counts, infile); 
     fclose(infile);
     free(targs);
     pthread_exit(NULL);
 }
 
 /*
  * main - handle command line, spawning one thread per file.
  */
 int main(int argc, char *argv[]) {
     /* Create the empty data structure. */
     word_count_list_t word_counts;
     init_words(&word_counts);
 
     if (argc <= 1) {
         /* Process stdin in a single thread. */
         count_words(&word_counts, stdin);
     } else {
         // Initialize threads
         pthread_t threads[argc - 1];
 
         // Go through each file
         for (int i = 1; i < argc; i++) {
             // Allocate memory for thread arguments
             thread_args_t *targs = malloc(sizeof(thread_args_t));
             if (targs == NULL) {
                 perror("malloc");
                 exit(1);
             }
 
             // Set up the thread arguments
             targs->word_counts = &word_counts;
             targs->filename = argv[i];
 
             // Create the thread
             if (pthread_create(&threads[i - 1], NULL, count_words_wrapper, (void *)targs)) {
                 perror("pthread_create did not succeed");
                 free(targs);
                 exit(1);
             }
         }
 
         // Join the threads to continue executing main
         for (int i = 0; i < argc - 1; i++) {
             pthread_join(threads[i], NULL);
         }
     }
 
     wordcount_sort(&word_counts, less_count);
     fprint_words(&word_counts, stdout);
 
     return 0;
 }