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

int exec_builtin(int argc, char** argv) {
    if (!argv || !argv[0]) return -1;

    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(argv[0], "cd") == 0) {
        if (argc != 2) {
            fprintf(stderr, "An error has occurred\n");
            exit(0);
        }
        if (chdir(argv[1]) != 0) {
            perror("chdir failed");
            exit(1);
        }

    } else if (strcmp(argv[0], "pwd") == 0) {
        char cwd[4094] = {0};
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd failed");
        } else {
            printf("%s\n", cwd);
        }
    } else if (strcmp(argv[0], "path") == 0) {
        for (int i = 1; argv[i]; i++) {
            size_t arg_len = strlen(argv[i]);
            if (argv[i][arg_len - 1] == '/') {
                argv[i][arg_len - 1] = '\0';
            }
        }
        char *new_path = join(argv + 1, argc - 1, ":");
        if (setenv("PATH", new_path, 1) == -1) {
            perror("setenv failed"); 
            exit(EXIT_FAILURE);
        }
        printf("%s\n", getenv("PATH"));
    } else {
        return -1;
    }
    return 0;
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
        if (exec_builtin(toks_cnt, toks) == -1) {
            exec_binary(toks, STDIN_FILENO, STDOUT_FILENO);
        }
        free_str_arr(toks, toks_cnt);
    }
    if (line) free(line);
    fclose(stream);
    return 0;
}
