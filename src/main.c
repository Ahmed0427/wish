#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"

#define EXIT_CMD_NOT_FOUND 127
#define EXIT_NOT_EXECUTABLE 126

int last_exit_code = 0;

void exec_binary(char** argv, int in_fd, int out_fd) {
    pid_t cpid = fork();
    if (cpid == -1) {
        return;
    } else if (cpid == 0) {
        size_t paths_cnt = 0;
        char** paths = split(getenv("PATH"), ":", &paths_cnt);
        for (int i = 0; paths[i]; i++) {
            char *toks[] = {paths[i], argv[0]};
            char *pathname = join(toks, 2, "/");
            if (pathname == NULL) return;
            if (access(pathname, X_OK) == 0) {
                dup2(in_fd, STDIN_FILENO);
                dup2(out_fd, STDOUT_FILENO);
                execv(pathname, argv);    
                perror("exec");
                return;
            } else if (access(pathname, F_OK) == 0) {
                perror("wish");
                exit(EXIT_NOT_EXECUTABLE);
            }
        }
        fprintf(stdout, "wish: command not found\n");
        exit(EXIT_CMD_NOT_FOUND);
    }

    int status;
    waitpid(cpid, &status, 0);

    if (WIFEXITED(status)) {
        last_exit_code = WEXITSTATUS(status);
    }
}

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
        if (argc == 1) printf("wish> ");
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
        exec_binary(toks, STDIN_FILENO, STDOUT_FILENO);
        free_str_arr(toks, toks_cnt);
    }
    if (line) free(line);
    fclose(stream);
    return 0;
}
