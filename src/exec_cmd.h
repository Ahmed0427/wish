#ifndef EXEC_CMD_H
#define EXEC_CMT_H

#define EXIT_CMD_NOT_FOUND 127
#define EXIT_NOT_EXECUTABLE 126

int exec_cmd(char** toks, size_t toks_cnt);

#endif
