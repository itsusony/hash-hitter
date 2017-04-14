#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "uthash.h"

#define MAX_BUFSIZE 10240

char *main_key = NULL, *sub_key = NULL;

typedef struct key2 {
    char* key;
    UT_hash_handle hh;
} KEY;

typedef struct item2 {
    char* val;
    KEY* keys;
    UT_hash_handle hh;
} ITEM;

typedef struct keyval {
    char* key;
    char* val;
    UT_hash_handle hh;
} KEYVAL;

ITEM* items = NULL;
pthread_mutex_t items_lock = PTHREAD_MUTEX_INITIALIZER;

KEYVAL* input_queue = NULL;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;


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
    KEY* p_key = NULL;
    HASH_FIND_STR(items, input_val, p_item);

    if (!p_item) {
        p_item = calloc(1, sizeof(ITEM));
        p_item->val = strdup(input_val);
        
        p_key = calloc(1, sizeof(KEY));
        p_key->key = strdup(input_key);
        HASH_ADD_KEYPTR(hh, p_item->keys, p_key->key, strlen(p_key->key), p_key);

        HASH_ADD_KEYPTR(hh, items, p_item->val, strlen(p_item->val), p_item);
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
    unsigned long sub_key_count = 0;
    unsigned long no_hit_sub_key_count = 0;

    ITEM *cur, *tmp;
    HASH_ITER(hh, items, cur, tmp) {
        KEY* p_k1 = NULL;
        HASH_FIND_STR(cur->keys, main_key, p_k1);
        if (p_k1) main_key_count++;

        KEY* p_k2 = NULL;
        HASH_FIND_STR(cur->keys, sub_key, p_k2);
        if (p_k2) {
            if (p_k1)
                sub_key_count++;
            else
                no_hit_sub_key_count++;
        }
    }
    system("clear");
    fprintf(stderr, "mainkey (%s):\n    %ld\n", main_key, main_key_count);
    fprintf(stderr, "subkey (%s):\n    %ld\n", sub_key, sub_key_count);
    fprintf(stderr, "no-hit subkey (%s):\n    %ld\n", sub_key, no_hit_sub_key_count);
    fprintf(stderr, "hit rate:\n    %.2f%%\n", main_key_count == 0 ? 0 : ((float)sub_key_count / (float)main_key_count * 100));
}

void write_queue(const char* input_key, const char* input_val) {
    KEYVAL* p_kv = calloc(1, sizeof(KEYVAL));
    p_kv->key = strdup(input_key);
    p_kv->val = strdup(input_val);

    pthread_mutex_lock(&queue_lock);
        HASH_ADD_KEYPTR(hh, input_queue, input_key, strlen(input_key), p_kv);
    pthread_mutex_unlock(&queue_lock);
}

void* pthread_p1_work(void* p) {
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
void* pthread_p2_work(void* p) {
    while(1) {
        KEYVAL* old_ptr = NULL;
        pthread_mutex_lock(&queue_lock);
            if (HASH_COUNT(input_queue) > 0) {
                old_ptr = input_queue;
                input_queue = NULL;
            }
        pthread_mutex_unlock(&queue_lock);

        if (old_ptr) {
            KEYVAL *cur, *tmp;

            HASH_ITER(hh, old_ptr, cur, tmp) {
                pthread_mutex_lock(&items_lock);
                write_item(cur->key, cur->val);
                pthread_mutex_unlock(&items_lock);

                HASH_DELETE(hh, old_ptr, cur);
                free(cur->key);
                free(cur->val);
                free(cur);
            }

            HASH_CLEAR(hh, old_ptr);
        }
        sleep(1);
    }
    return NULL;
}

void* pthread_p3_work(void* p) {
    while(1) {
        pthread_mutex_lock(&items_lock);
        summary();
        pthread_mutex_unlock(&items_lock);
        sleep(1);
    }
}

int main(int argc, char*argv[]) {
    if (argc != 3 || !strlen(argv[1]) || !strlen(argv[2])) {
        fprintf(stderr, "usage: ./view2 MAINKEY SUBKEY\n");
        return 1;
    }

    main_key = strdup(argv[1]);
    sub_key  = strdup(argv[2]);

    pthread_t p1, p2, p3;
    pthread_create(&p1, NULL, (void*)pthread_p1_work, (void*)NULL);
    pthread_create(&p2, NULL, (void*)pthread_p2_work, (void*)NULL);
    pthread_create(&p3, NULL, (void*)pthread_p3_work, (void*)NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    return 0;
}