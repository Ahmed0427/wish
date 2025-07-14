#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

#include "exec_cmd.h"
#include "utils.h"

int run(struct cmd *cmd, int in_fd, int out_fd) {
    if (cmd == NULL) return 0;

    int argc = 0;
    if (cmd->argv) {
        for (int i = 0; cmd->argv[i]; i++) {
            argc++;
        }
    }
    switch (cmd->type) {
        case EXEC: {
            if (!cmd->argv || !cmd->argv[0]) return -1;
            if (strcmp(cmd->argv[0], "exit") == 0) {
                if (argc != 1) {
                    fprintf(stderr, "An error has occurred\n");
                }
                exit(0);
            } else if (strcmp(cmd->argv[0], "cd") == 0) {
                if (argc != 2) {
                    fprintf(stderr, "An error has occurred\n");
                    exit(0);
                }
                if (chdir(cmd->argv[1]) != 0) {
                    perror("chdir failed");
                    exit(1);
                }

            } else if (strcmp(cmd->argv[0], "pwd") == 0) {
                char cwd[4094] = {0};
                if (getcwd(cwd, sizeof(cwd)) == NULL) {
                    perror("getcwd failed");
                } else {
                    printf("%s\n", cwd);
                }
            } else if (strcmp(cmd->argv[0], "path") == 0) {
                for (int i = 1; i < argc; i++) {
                    size_t arg_len = strlen(cmd->argv[i]);
                    if (cmd->argv[i][arg_len - 1] == '/') {
                        cmd->argv[i][arg_len - 1] = '\0';
                    }
                    if (cmd->argv[i][0] != '/') {
                        char cwd[4094] = {0};
                        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                            perror("getcwd failed");
                            return 1;
                        } else {
                            strcat(cwd, "/");
                            strcat(cwd, cmd->argv[i]);
                            free(cmd->argv[i]);
                            cmd->argv[i] = strdup(cwd);
                        }
                    }
                }
                char *new_path = join(cmd->argv + 1, argc - 1, ":");
                if (setenv("PATH", new_path, 1) == -1) {
                    perror("setenv failed"); 
                    exit(EXIT_FAILURE);
                }
            } else {
                pid_t cpid = fork();
                if (cpid == -1) {
                    return -1;
                } else if (cpid == 0) {
                    if (dup2(out_fd, STDOUT_FILENO) < 0) {
                        perror("dup2 failed");
                        exit(1);
                    }
                    if (dup2(in_fd, STDIN_FILENO) < 0) {
                        perror("dup2 failed");
                        exit(1);
                    }
                    execvp(cmd->argv[0], cmd->argv);
                    fprintf(stderr, "An error has occurred\n");
                    exit(-1);
                }
                int wstatus;
                wait(&wstatus);
                if (WIFEXITED(wstatus)) {
                    return WEXITSTATUS(wstatus);
                }
                return -1;
            }
            break;
        }
        case REDIR: {
            int fd;
            if (cmd->redir_type == 0) { 
                fd = open(cmd->redir_file, O_RDONLY);
                if (fd < 0) {
                    perror("open input failed");
                    exit(1);
                }
                run(cmd->left, fd, out_fd);
            } else { 
                fd = open(cmd->redir_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd < 0) {
                    perror("open output failed");
                    exit(1);
                }
                run(cmd->left, in_fd, fd);
            }
            close(fd);
            break;
        }
        case ASYNC: {
            run(cmd->left, in_fd, out_fd);        
            run(cmd->right, in_fd, out_fd);        
            break;
        }
    }
    return 0;
}

int exec_cmd(struct cmd *cmd) {
    return run(cmd, STDIN_FILENO, STDOUT_FILENO);
}
