#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "exec_cmd.h"
#include "utils.h"

int exec_cmd(char** toks, size_t toks_cnt) {
    size_t argc = toks_cnt;
    int has_redirection = 0; 
    for (size_t i = 0; i < toks_cnt; i++) {
        if (strcmp(toks[i], ">") == 0) {
            has_redirection = 1;
        }
    }
    int f_out = STDOUT_FILENO;
    if (has_redirection) {
        if (toks_cnt < 2 || strcmp(toks[toks_cnt - 2], ">") != 0) {
            fprintf(stderr, "An error has occurred\n");
            return 0;
        } else {
            f_out = open(toks[toks_cnt - 1], O_CREAT | O_TRUNC | O_RDWR, 0664);
            if (f_out == -1) {
                perror("open failed");
                return 0;
            }
            toks[toks_cnt - 2] = '\0';
            argc -= 2;
        }
    }
    
    if (!toks[0]) return 0;

    if (strcmp(toks[0], "exit") == 0) {
        if (argc != 1) {
            fprintf(stderr, "An error has occurred\n");
            exit(0);
        }
        exit(0);
    } else if (strcmp(toks[0], "cd") == 0) {
        if (argc != 2) {
            fprintf(stderr, "An error has occurred\n");
            exit(0);
        }
        if (chdir(toks[1]) != 0) {
            perror("chdir failed");
            exit(1);
        }
    } else if (strcmp(toks[0], "pwd") == 0) {
        char cwd[4094] = {0};
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd failed");
        } else {
            printf("%s\n", cwd);
        }
    } else if (strcmp(toks[0], "path") == 0) {
        for (size_t i = 1; i < argc; i++) {
            size_t arg_len = strlen(toks[i]);
            if (toks[i][arg_len - 1] == '/') {
                toks[i][arg_len - 1] = '\0';
            }
            if (toks[i][0] != '/') {
                char cwd[4094] = {0};
                if (getcwd(cwd, sizeof(cwd)) == NULL) {
                    perror("getcwd failed");
                    return 1;
                } else {
                    strcat(cwd, "/");
                    strcat(cwd, toks[i]);
                    free(toks[i]);
                    toks[i] = strdup(cwd);
                }
            }

        }
        char *new_path = join(toks + 1, argc - 1, ":");
        if (setenv("PATH", new_path, 1) == -1) {
            perror("setenv failed"); 
            exit(EXIT_FAILURE);
        }
    } else {
        pid_t cpid = fork();
        if (cpid == -1) {
            return 0;
        } else if (cpid == 0) {
            size_t paths_cnt = 0;
            char** paths = split(getenv("PATH"), ":", &paths_cnt);
            for (int i = 0; paths[i]; i++) {
                char *path_arr[] = {paths[i], toks[0]};
                char *pathname = join(path_arr, 2, "/");
                if (toks[0][0] == '/') pathname = toks[0];
                if (pathname == NULL) return 0;
                if (access(pathname, X_OK) == 0) {
                    dup2(f_out, STDOUT_FILENO);
                    execv(pathname, toks);    
                    perror("exec");
                    return 0;
                } else if (access(pathname, F_OK) == 0) {
                    perror("wish");
                    exit(EXIT_NOT_EXECUTABLE);
                }
            }
            fprintf(stderr, "An error has occurred\n");
            exit(EXIT_CMD_NOT_FOUND);
        }

        int status;
        waitpid(cpid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    return 0;
}
