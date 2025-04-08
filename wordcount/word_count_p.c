/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"


//The difference from PINTO LIST is that we are using struct with lock
void init_words(word_count_list_t *wclist) {
    list_init(&wclist->lst);
    pthread_mutex_init(&wclist->lock, NULL);
}

//want to use lock to protect the list from parallel access and race conditions
size_t len_words(word_count_list_t *wclist) {
    size_t len = 0; 
    pthread_mutex_lock(&wclist->lock);
    word_count_t *cur; 
    for (cur = list_begin(&wclist->lst); cur != list_end(&wclist->lst);
         cur = list_next(cur)) {
        len++;
    }
    pthread_mutex_unlock(&wclist->lock);
    //test
    return 0;
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
    /* TODO */
    return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
    /* TODO */
    return NULL;
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
    /* TODO */
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
    /* TODO */
}
