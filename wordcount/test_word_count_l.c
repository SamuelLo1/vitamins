#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "word_count.h"

void test_init_words() {
    word_count_list_t word_counts;
    init_words(&word_counts);
}

void test_len_words() {
    word_count_list_t wclist;
    init_words(&wclist);

    

}

// void test_find_word() {
//     word_count_list_t wclist;
//     init_words(&wclist);

//     add_word(&wclist, strdup("test"));
//     word_count_t *wc = find_word(&wclist, "test");

//     if (wc != NULL && strcmp(wc->word, "test") == 0) {
//         printf("test_find_word: PASSED\n");
//     } else {
//         printf("test_find_word: FAILED\n");
//     }
// }

// void test_add_word() {
//     word_count_list_t wclist;
//     init_words(&wclist);

//     add_word(&wclist, strdup("example"));
//     word_count_t *wc = find_word(&wclist, "example");

//     if (wc != NULL && strcmp(wc->word, "example") == 0 && wc->count == 1) {
//         printf("test_add_word: PASSED\n");
//     } else {
//         printf("test_add_word: FAILED\n");
//     }
// }

int main() {
    test_init_words();
   // test_len_words();
   // test_find_word();
   // test_add_word();
    return 0;
}