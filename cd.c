#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "builtin.h"

static char **dir_stack = NULL;
static int size = 0;
static int top = 0;
static char *home = NULL;

int sh_chdir(const char *path)
{
  extern int dir_changed;
  int status = chdir(path);
  if (!status) dir_changed = 1;
  return status;
}

void cd(cmd *command)
{
  char *dir;
  if (command->next == 1) { // just cd, then chdir to home directory
    sh_chdir(home);
    dir_stack[top-1] = realloc(dir_stack[top-1], (strlen(home) + 1) * sizeof(char));
    strcpy(dir_stack[top-1], home);
  } else if (sh_chdir(command->argv[1]) < 0) {
    perror("shell: cd");
    return;
  } else {
    dir_stack[top-1] = realloc(dir_stack[top-1], (strlen(sh_getcwd(command->argv[1]))+1) * sizeof(char));
    strcpy(dir_stack[top-1], sh_getcwd(command->argv[1]));
  }
}

void pwd(cmd *command)
{
  char *s = sh_getcwd();
  printf("%s\n", s);
  free(s);
}

void dir_init()
{
  size = 10;
  dir_stack = malloc(size * sizeof(char *));
  dir_stack[top++] = sh_getcwd();
  home = getenv("HOME");
}

void pushd(cmd *command)
{
  char *dir;
  if (top == size)
    dir_stack = enlarge_ptr(dir_stack, &size, sizeof(char *));
  if (command->next == 1) { // no argument
    if (top > 1) {
      dir = dir_stack[top-1];
      dir_stack[top-1] = dir_stack[top-2];
      dir_stack[top-2] = dir;
      sh_chdir(dir_stack[top-1]);
    } else
      fprintf(stderr, "shell: pushd: no other directory\n");
  } else {
    if (access(command->argv[1], F_OK) < 0) {
      perror("shell: pushd");
      return;
    }
    sh_chdir(command->argv[1]);
    dir_stack[top] = malloc((strlen(sh_getcwd(command->argv[1]))+1) * sizeof(char));
    strcpy(dir_stack[top++], sh_getcwd(command->argv[1]));
    sh_chdir(dir_stack[top-1]);
  }
}

void popd(cmd *command)
{
  if (top == 1)
    fprintf(stderr, "shell: popd: directory stack empty\n");
  else {
    free(dir_stack[--top]);
    printf("%s\n", dir_stack[top-1]);
    sh_chdir(dir_stack[top-1]);
  }
}
