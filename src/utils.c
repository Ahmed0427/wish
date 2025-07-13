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

int read_file(const char* path, unsigned char** data, int* size) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        *data = NULL;
        *size = 0;
        return -1;
    }

    int fsize = lseek(fd, 0, SEEK_END);
    if (fsize == -1) {
        perror("lseek error");
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);

    unsigned char* buf = malloc(fsize);
    if (!buf) {
        close(fd);
        return -1;
    }

    int rb = read(fd, buf, fsize);
    if (rb != fsize) {
        perror("read error");
        free(buf);
        close(fd);
        return -1;
    }

    *size = fsize;
    *data = buf;
    close(fd);
    return 0;
}

int write_file(const char* path, const unsigned char* data,
               size_t size, int mode) {

    int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, mode);
    if (fd == -1) {
        fprintf(stderr, "'%s' failed to write: %s\n", path, strerror(errno));
        return -1;
    }

    ssize_t wb = write(fd, data, size);
    close(fd);

    return (wb == (ssize_t)size) ? 0 : -1;
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
