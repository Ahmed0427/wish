#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void free_str_arr(char** arr, size_t cnt) {
    for (size_t i = 0; i < cnt; i++) {
        free(arr[i]);
    }
    free(arr);
}

char** split_str(const char* org_str, const char* del, size_t* toks_cnt) {
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
            toks = realloc(toks, sizeof(char*) * cap);
            if (!toks) {
                free(str);
                free_str_arr(toks, sz);
                *toks_cnt = 0;
                return NULL;
            }
        }

        toks[sz] = strdup(tok);
        if (!toks[sz]) {
            free(str);
            free_str_arr(toks, sz);
            *toks_cnt = 0;
            return NULL;
        }

        tok = strtok(NULL, del);
        sz++;
    }

    toks = realloc(toks, sizeof(char*) * (sz + 1));
    if (!toks) {
        free(str);
        free_str_arr(toks, sz);
        *toks_cnt = 0;
        return NULL;
    }
    toks[sz] = NULL;

    free(str);
    *toks_cnt = sz;
    return toks;
}


int main(int argc, char* argv[]) {
    char *line = NULL;
    size_t line_len = 0;
    FILE *stream = stdin;
    if (argc == 2) {
        stream = fopen(argv[1], "r");
        if (stream == NULL) {
            perror("fopen failed");
            exit(EXIT_FAILURE);
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (argc == 1) printf("wish> ");
        ssize_t read = getline(&line, &line_len, stream);
        if (read == -1) {
            fclose(stream);
            if (line) free(line);
            exit(0);
        }
        size_t toks_cnt = 0;
        line[read - 1] = '\0'; 
        char** toks = split_str(line, " \t", &toks_cnt);
        if (toks == NULL) {
            fclose(stream);
            if (line) free(line);
            perror("split_str failed");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < toks_cnt; i++) {
            printf("'%s' ", toks[i]);
        }
        printf("\n");

        free_str_arr(toks, toks_cnt);
    }

    if (line) free(line);
    fclose(stream);
    return 0;
}
