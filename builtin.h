#ifndef BUILTIN_H
#define BUILTIN_H
#include <stdio.h>
#include <unistd.h>
#include "type.h"

extern int BUILTIN_N;
typedef void builtin_cmd(cmd *command);
extern const char *builtin_list[];

void dir_init();
builtin_cmd cd;
builtin_cmd pwd;
builtin_cmd pushd;
builtin_cmd popd;
builtin_cmd dirs;

builtin_cmd sh_exit;
extern builtin_cmd *builtins[];

// helper funcs
void *enlarge_ptr(void *ptr, int *size, int type_size);
char *sh_getcwd();
int sh_strtoi(char *s, int *err);
#endif
