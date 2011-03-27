#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "builtin.h"

int BUILTIN_N = 6;
const char *builtin_list[] = {
  "cd",
  "pwd",
  "dirs",
  "pushd",
  "popd",
  "exit"
};
builtin_cmd *builtins[] = {
  cd, 
  pwd,
  dirs,
  pushd,
  popd,
  sh_exit
};

void sh_exit(cmd *command)
{
  int rt_val = 0;
  if (command->next > 1) {
    char *ptr = NULL;
    rt_val = strtol(command->argv[1], &ptr, 10);
    if (*ptr) {
      fprintf(stderr, "shell: exit: %s: numeric argument expected\n", command->argv[1]);
      rt_val = 127;
    }
  }
  //TODO: handle background processes
  exit(rt_val);
}

void *enlarge_ptr(void *ptr, int *size, int type_size)
{
  (*size) *= 2;
  return realloc(ptr, *size * type_size);
}

int sh_strtoi(char *s, int *err)
{
  char *endptr;
  int result;
  result = strtol(s, &endptr, 10);
  *err = (*endptr) ? -1 : 0;
  return result;
}
