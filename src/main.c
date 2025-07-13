#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"
#include "exec_cmd.h"

int last_exit_code = 0;

int main(int argc, char** argv) {
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

    if (setenv("PATH", "/bin:/usr/bin", 0) == -1) {
        perror("setenv failed"); 
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (argc == 1) {
            char cwd[4094] = {0};
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                perror("getcwd failed");
                exit(1);
            }
            printf("[%s]\n", cwd);
            printf("> ");
        }
        ssize_t read = getline(&line, &line_len, stream);
        if (read == -1) {
            fclose(stream);
            if (line) free(line);
            exit(0);
        }
        size_t toks_cnt = 0;
        line[read - 1] = '\0'; 
        char** toks = split(line, " \t",&toks_cnt);
        if (toks == NULL) {
            fclose(stream);
            if (line) free(line);
            perror("tokenize failed");
            exit(EXIT_FAILURE);
        }
        last_exit_code = exec_cmd(toks, toks_cnt);
        free_str_arr(toks, toks_cnt);
    }
    if (line) free(line);
    fclose(stream);
    return 0;
}
