#ifndef UTILS_T
#define UTILS_T

void free_str_arr(char** arr, int cnt);

void print_str_arr(char** arr, int cnt);

int read_file(const char* path, unsigned char** data, int* size);

int write_file(const char* path, const unsigned char* data, size_t size, int mode);

char** split(char* str, const char* del, size_t* toks_cnt);

char* join(char** toks, size_t count, const char* del);

#endif
