#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

void free_str_arr(char** arr, int cnt) {
    for (int i = 0; i < cnt; i++) {
        free(arr[i]);
    }
    free(arr);
}

void print_str_arr(char** arr, int cnt) {
    for (int i = 0; i < cnt; i++) {
        printf("%s\n", arr[i]);
    }
}

char** split(char* org_str, const char* del, size_t* toks_cnt) {
    char* str = strdup(org_str);

    size_t cap = 1, sz = 0;
    char** toks = malloc(sizeof(char*) * cap);
    if (!toks) {
        *toks_cnt = 0;
        return NULL;
    }

    char* tok = strtok(str, del);
    while (tok != NULL) {
        if (sz >= cap) {
            cap *= 2;
            char** tmp = realloc(toks, sizeof(char*) * cap);
            if (!tmp) {
                free_str_arr(toks, sz);
                *toks_cnt = 0;
                return NULL;
            }
            toks = tmp;
        }

        toks[sz] = strdup(tok);
        if (!toks[sz]) {
            free_str_arr(toks, sz);
            *toks_cnt = 0;
            return NULL;
        }

        sz++;
        tok = strtok(NULL, del);
    }

    char** tmp = realloc(toks, sizeof(char*) * (sz + 1));
    if (!tmp) {
        free(toks);
        *toks_cnt = 0;
        return NULL;
    }
    toks = tmp;
    toks[sz] = NULL;

    *toks_cnt = sz;
    free(str);
    return toks;
}

char* join(char** toks, size_t count, const char* del) {
    if (count == 0) return strdup("");

    size_t del_len = strlen(del);
    size_t total_len = 0;

    for (size_t i = 0; i < count; ++i) {
        if (toks[i] != NULL) total_len += strlen(toks[i]);
        if (i < count - 1) total_len += del_len;
    }

    char* result = malloc(total_len + 1);
    if (!result) return NULL;

    result[0] = '\0';
    for (size_t i = 0; i < count; ++i) {
        strcat(result, toks[i]);
        if (i < count - 1) strcat(result, del);
    }

    return result;
}
