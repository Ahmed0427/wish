#ifndef PARSER_H
#define PARSER_H

enum node_type { EXEC, PIPE, SEQ, REDIR, ASYNC};

struct cmd {
    enum node_type type;
    struct cmd *left;
    struct cmd *right;
    char **argv;       // only for EXEC
    char *redir_file;  // only for REDIR
    int redir_type;    // 0 = input (<), 1 = output (>)
};

struct cmd *parse(char **toks);
void print_cmd(struct cmd *cmd, int indent);
void free_cmd(struct cmd *cmd);

#endif
