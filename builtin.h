#ifndef BUILTIN_H
#define BUILTIN_H
#include <stdio.h>
#include <unistd.h>
#include "type.h"

#define BUILTIN_N 3
typedef void builtin_cmd(cmd *command);
extern const char *builtin_list[];

builtin_cmd cd;
builtin_cmd pwd;
builtin_cmd pushd;
builtin_cmd popd;

builtin_cmd sh_exit;
extern builtin_cmd *builtins[];

// helper funcs
void *enlarge_ptr(void *ptr, int *size, int type_size);
char *sh_getcwd();
#endif
