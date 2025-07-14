#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parser.h"

int is_redir(const char *s) {
    return s && (strcmp(s, "<") == 0 || strcmp(s, ">") == 0);
}

struct cmd *parse_exec_or_redir(char **toks, int *i) {
    struct cmd *cmd = malloc(sizeof(*cmd));
    cmd->type = EXEC;
    cmd->left = cmd->right = NULL;
    cmd->argv = calloc(32, sizeof(char*));
    cmd->redir_file = NULL;
    cmd->redir_type = -1;

    int argc = 0;

    while (toks[*i] && strcmp(toks[*i], ";") != 0 &&
           strcmp(toks[*i], "&") != 0 && strcmp(toks[*i], "|") != 0) {

        if (is_redir(toks[*i])) {
            struct cmd *redir = malloc(sizeof(*redir));
            redir->type = REDIR;
            redir->left = cmd;
            redir->right = NULL;
            redir->redir_type = (strcmp(toks[*i], "<") == 0) ? 0 : 1;
            (*i)++;  // consume < or >
            if (toks[*i]) redir->redir_file = strdup(toks[*i]);
            else {
                free(redir);
                free(cmd->argv);
                free(cmd);
                return NULL;
            }
            (*i)++;  // consume filename
            
            if (toks[*i] || argc == 0) return NULL;
            return redir;
        }

        cmd->argv[argc++] = strdup(toks[*i]);
        (*i)++;
    }

    cmd->argv[argc] = NULL;
    return cmd;
}

struct cmd *parse_pipe(char **toks, int *i) {
    struct cmd *cmd = parse_exec_or_redir(toks, i);
    if (cmd == NULL) return NULL;
    while (toks[*i] && strcmp(toks[*i], "|") == 0) {
        (*i)++;  // consume |
        struct cmd *pipe = malloc(sizeof(*pipe));
        pipe->type = PIPE;
        pipe->left = cmd;
        pipe->right = parse_pipe(toks, i);
        pipe->argv = NULL;
        cmd = pipe;
    }
    return cmd;
}

struct cmd *parse_async(char **toks, int* i) {
    struct cmd *cmd = parse_pipe(toks, i);
    if (cmd == NULL) return NULL;
    while (toks[*i] && strcmp(toks[*i], "&") == 0) {
        (*i)++;  // consume &
        struct cmd *seq = malloc(sizeof(*seq));
        seq->type = ASYNC;
        seq->left = cmd;
        seq->right = parse_async(toks, i);
        seq->argv = NULL;
        cmd = seq;
    }
    return cmd;
}

struct cmd *parse_seq(char **toks, int *i) {
    struct cmd *cmd = parse_async(toks, i);
    if (cmd == NULL) return NULL;
    while (toks[*i] && strcmp(toks[*i], ";") == 0) {
        (*i)++;  // consume ;
        struct cmd *seq = malloc(sizeof(*seq));
        seq->type = SEQ;
        seq->left = cmd;
        seq->right = parse_seq(toks, i);
        seq->argv = NULL;
        cmd = seq;
    }
    return cmd;
}

struct cmd *parse(char **toks) {
    int i = 0;
    return parse_seq(toks, &i);
}

void print_cmd(struct cmd *cmd, int indent) {
    if (!cmd) return;
    for (int i = 0; i < indent; i++) printf("  ");

    switch (cmd->type) {
        case EXEC:
            printf("EXEC: ");
            for (char **arg = cmd->argv; *arg; arg++) {
                printf("%s ", *arg);
            }
            printf("\n");
            break;

        case PIPE:
            printf("PIPE:\n");
            print_cmd(cmd->left, indent + 1);
            print_cmd(cmd->right, indent + 1);
            break;

        case SEQ:
            printf("SEQ:\n");
            print_cmd(cmd->left, indent + 1);
            print_cmd(cmd->right, indent + 1);
            break;

        case REDIR:
            printf("REDIR (%s):\n", cmd->redir_type == 0 ? "<" : ">");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("FILE: %s\n", cmd->redir_file);
            print_cmd(cmd->left, indent + 1);
            break;

        default:
            printf("UNKNOWN\n");
            break;
    }
}

void free_cmd(struct cmd *cmd) {
    if (!cmd) return;

    switch (cmd->type) {
        case EXEC:
            if (cmd->argv) {
                for (char **arg = cmd->argv; *arg; arg++) {
                    free(*arg);      
                }
                free(cmd->argv);    
            }
            free(cmd);
            break;

        case REDIR:
            free(cmd->redir_file);   
            free_cmd(cmd->left);
            free(cmd);
            break;

        case PIPE:
        case SEQ:
        case ASYNC:
            free_cmd(cmd->left);
            free_cmd(cmd->right);
            free(cmd);
            break;
    }
}
