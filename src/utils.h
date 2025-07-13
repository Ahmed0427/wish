#ifndef UTILS_T
#define UTILS_T

void free_str_arr(char** arr, int cnt);

void print_str_arr(char** arr, int cnt);

char** split(char* str, const char* del, size_t* toks_cnt);

char* join(char** toks, size_t count, const char* del);

#endif
