#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "builtin.h"

const char *builtin_list[] = {"cd", "pwd", "exit"};
builtin_cmd *builtins[] = {&cd, &pwd, &sh_exit};

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

char *sh_getcwd()
{
  int size = 20;
  char *s = malloc(size * sizeof(char));
  char *home = getenv("HOME");
  char *ptr, *new_s = NULL;
  while (getcwd(s, size) == NULL)
    if (errno = ERANGE) {
      s = enlarge_ptr(s, &size, sizeof(char));
    } else {
      perror("shell");
      free(s);
      return NULL;
    }
  if (s && (ptr = strstr(s, home))) {
    int i;
    ptr += strlen(home);
    new_s = malloc((strlen(ptr) + 2) * sizeof(char));
    new_s[0] = '~';
    new_s[1] = '\0';
    strcat(new_s, ptr);
    free(s);
    return new_s;
  }
  return s;
}
