#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "builtin.h"

void cd(cmd *command)
{
  extern dir_changed;
  if (command->next == 1) // just cd, then chdir to home directory
    chdir(getenv("HOME"));
  else if (chdir(command->argv[1]) < 0) {
    perror("shell: cd");
    return;
  }
  dir_changed = 1;
}

void pwd(cmd *command)
{
  char *s = sh_getcwd();
  printf("%s\n", s);
  free(s);
}
