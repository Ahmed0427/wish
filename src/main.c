#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"
#include "parser.h"
#include "exec_cmd.h"

int last_exit_code = 0;

int main(int argc, char** argv) {
    char *line = NULL;
    size_t line_len = 0;
    FILE *stream = stdin;
    if (argc == 2) {
        stream = fopen(argv[1], "r");
        if (stream == NULL) {
            fprintf(stderr, "An error has occurred\n");
            exit(EXIT_FAILURE);
        }
    } else if (argc > 2) {
        fprintf(stderr, "An error has occurred\n");
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
        char** toks = split(line, " \t", &toks_cnt);
        if (toks == NULL) {
            fclose(stream);
            if (line) free(line);
            perror("split failed");
            exit(EXIT_FAILURE);
        }

        struct cmd *cmd = parse(toks);
        // print_cmd(cmd, 0);
        if (!cmd) fprintf(stderr, "An error has occurred\n");
        else exec_cmd(cmd);
        free_cmd(cmd);

        free_str_arr(toks, toks_cnt);
    }
    if (line) free(line);
    fclose(stream);
    return 0;
}
