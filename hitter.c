#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "include/uthash.h"
#include "include/utarray.h"

#define MAX_BUFSIZE 10240

char *main_key = NULL;

typedef struct key {
    char* key;
    UT_hash_handle hh;
} KEY;

typedef struct subkey_counter {
    char* key;
    unsigned long count;
    UT_hash_handle hh;
} SUBKEYCOUNTER;

typedef struct item {
    char* val;
    KEY* keys;
    UT_hash_handle hh;
} ITEM;

typedef struct keyval {
    char* key;
    char* val;
} KEYVAL;


ITEM* items = NULL;
pthread_mutex_t items_lock = PTHREAD_MUTEX_INITIALIZER;

UT_array *input_queue = NULL;
UT_icd kv_icd = {sizeof(KEYVAL), NULL, NULL, NULL};
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

void init_input_queue() {
    input_queue = NULL;
    utarray_new(input_queue, &kv_icd);
}


// return 1 error 0 ok
int parse(const char* input_buf, char* output_key, char* output_val) {
    size_t buf_len = strlen(input_buf);
    if (buf_len <= 0) return 1;

    char *p_colon   = strchr(input_buf, ':');
    char *p_newline = strchr(input_buf, '\n');
    if (!p_colon || !p_newline) return 1;

    long key_len = p_colon - input_buf;
    if (key_len <= 0) return 1;
    
    long val_len = strlen(input_buf) - key_len - 1 -1;
    if (val_len <= 0) return 1;

    strncat(output_key, input_buf, key_len);
    strncat(output_val, input_buf+key_len+1, val_len);

    return 0;
}

// return: 1 error 0 ok
int write_item(const char* input_key, const char* input_val) {
    ITEM* p_item = NULL;
    KEY* p_key   = NULL;
    HASH_FIND_STR(items, input_val, p_item);

    if (!p_item) {
        if (strcmp(main_key, input_key) == 0) { // just allow first key register when input_key === main_key
            p_item = calloc(1, sizeof(ITEM));
            p_item->val = strdup(input_val);
            p_item->keys = NULL;
            
            p_key = calloc(1, sizeof(KEY));
            p_key->key = strdup(input_key);
            HASH_ADD_KEYPTR(hh, p_item->keys, p_key->key, strlen(p_key->key), p_key);

            HASH_ADD_KEYPTR(hh, items, p_item->val, strlen(p_item->val), p_item);
        } else {
            return 1;
        }
    } else { // exist val
        HASH_FIND_STR(p_item->keys, input_key, p_key);
        if (!p_key) {
            p_key = calloc(1, sizeof(KEY));
            p_key->key = strdup(input_key);
            HASH_ADD_KEYPTR(hh, p_item->keys, p_key->key, strlen(p_key->key), p_key);
        }
    }

    return 0;
}

void summary() {
    unsigned long main_key_count = 0;
    ITEM *cur, *tmp;
    KEY *sub_cur, *sub_tmp;
    SUBKEYCOUNTER *subkey_counter = NULL;

    HASH_ITER(hh, items, cur, tmp) {
        KEY* p_k1 = NULL;
        HASH_FIND_STR(cur->keys, main_key, p_k1);
        if (p_k1) {
            main_key_count++;
        }

        HASH_ITER(hh, cur->keys, sub_cur, sub_tmp) {
            if (strcmp(sub_cur->key, main_key) != 0) {
                SUBKEYCOUNTER* subkey = NULL;
                HASH_FIND_STR(subkey_counter, sub_cur->key, subkey);
                if (subkey) {
                    subkey->count++;
                } else {
                    subkey = malloc(sizeof(SUBKEYCOUNTER));
                    subkey->key = strdup(sub_cur->key);
                    subkey->count = 1;
                    HASH_ADD_KEYPTR(hh, subkey_counter, sub_cur->key, strlen(sub_cur->key), subkey);

                }
            }
        }
    }

    system("clear");
    fprintf(stderr, "mainkey (%s):\t%ld\n\n",       main_key, main_key_count);
    SUBKEYCOUNTER *cnt_cur, *cnt_tmp;
    HASH_ITER(hh, subkey_counter, cnt_cur, cnt_tmp) {
        fprintf(stderr, "subkey (%s):\t%ld\n", cnt_cur->key, cnt_cur->count);
        HASH_DELETE(hh, subkey_counter, cnt_cur);
        free(cnt_cur->key);
        free(cnt_cur);
    }
    HASH_CLEAR(hh, subkey_counter);
}

void write_queue(const char* input_key, const char* input_val) {
    KEYVAL p_kv;
    p_kv.key = strdup(input_key);
    p_kv.val = strdup(input_val);

    pthread_mutex_lock(&queue_lock);
        utarray_push_back(input_queue, &p_kv);
    pthread_mutex_unlock(&queue_lock);
}

void* pthread_p_pipeinput_work(void* p) {
    char input_buf[MAX_BUFSIZE] = "",
         output_key[MAX_BUFSIZE]="",
         output_val[MAX_BUFSIZE]="";
    while(fgets(input_buf, sizeof(input_buf), stdin) != NULL) {
        if (parse(input_buf, output_key, output_val)) continue;
        write_queue(output_key, output_val);
        memset(input_buf, 0, sizeof(input_buf));
        memset(output_key, 0, sizeof(output_key));
        memset(output_val, 0, sizeof(output_val));
    }
    return NULL;
}
void* pthread_p_queueprocess_work(void* p) {
    while(1) {
        UT_array *old_input_queue = NULL;
        pthread_mutex_lock(&queue_lock);
            if (utarray_len(input_queue) > 0) {
                old_input_queue = input_queue;
                init_input_queue();
            }
        pthread_mutex_unlock(&queue_lock);

        if (old_input_queue) {
            KEYVAL* p = NULL;
            for(
                p=(KEYVAL*)utarray_front(old_input_queue);
                p!=NULL;
                p=(KEYVAL*)utarray_next(old_input_queue,p)
            ) {
                pthread_mutex_lock(&items_lock);
                    write_item(p->key, p->val);
                    free(p->key);
                    free(p->val);
                pthread_mutex_unlock(&items_lock);
            }
            utarray_clear(old_input_queue);
            utarray_free(old_input_queue);
        }
        sleep(1);
    }
    return NULL;
}

void* pthread_p_summary_work(void* p) {
    while(1) {
        pthread_mutex_lock(&items_lock);
            summary();
        pthread_mutex_unlock(&items_lock);
        sleep(1);
    }
}

int main(int argc, char*argv[]) {
    if (argc != 2 || !strlen(argv[1])) {
        fprintf(stderr, "usage: ./hitter MAINKEY\n");
        return 1;
    }

    main_key = strdup(argv[1]);

    init_input_queue();

    pthread_t p_pipeinput, p_queueprocess, p_summary;
    pthread_create(&p_pipeinput,    NULL, (void*)pthread_p_pipeinput_work,    (void*)NULL);
    pthread_create(&p_queueprocess, NULL, (void*)pthread_p_queueprocess_work, (void*)NULL);
    pthread_create(&p_summary,      NULL, (void*)pthread_p_summary_work,      (void*)NULL);

    pthread_join(p_pipeinput,    NULL);
    pthread_join(p_queueprocess, NULL);
    pthread_join(p_summary,      NULL);
    return 0;
}
