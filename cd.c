#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "builtin.h"
#include "linked_list.h"

//static char **dir_stack = NULL;
static linked_list dir_stack;
/* static int size = 0; */
/* static int top = 0; */
static char *home = NULL;

int sh_chdir(const char *path)
{
  extern int dir_changed;
  int status = chdir(path);
  if (!status) dir_changed = 1;
  return status;
}
char *sh_getcwd()
{
  int size = 20;
  char *s = malloc(size * sizeof(char));
  char *ptr, *new_s = NULL;
  while (getcwd(s, size) == NULL)
    if (errno = ERANGE) {
      s = enlarge_ptr(s, &size, sizeof(char));
    } else {
      perror("shell");
      free(s);
      return NULL;
    }
  return s;
}
static print_dir_stack(FILE *file)
{
  int count = 0;
  l_node *ptr = dir_stack->next;
  while (ptr) {
    fprintf(file, "%s%s", (count++ ? " " : ""), (char *)(ptr->val));
    ptr = ptr->next;
  }
  fputc('\n', file);
}
void dir_init()
{
  dir_stack = linked_list_init();
  push(&dir_stack, sh_getcwd());
  home = getenv("HOME");
}
void cd(cmd *command)
{
  if (command->next == 1) { // just cd, then chdir to HOME
    sh_chdir(home);
    dir_stack->next->val = realloc(dir_stack->next->val, (strlen(home)+1) * sizeof(char));
    strcpy(dir_stack->next->val, home);
  } else if (sh_chdir(command->argv[1]) != 0)
    fprintf(stderr, "shell: cd: %s: %s\n", command->argv[1], strerror(errno));
  else {
    free(dir_stack->next->val);
    dir_stack->next->val = sh_getcwd();
  }
}
void pwd(cmd *command)
{
  fprintf(stdout, "%s\n", (char *)dir_stack->next->val);
}
void dirs(cmd *command)
{
  print_dir_stack(stdout);
}
void pushd(cmd *command)
{
  static char *help_msg = "pushd: usage: pushd [-n] [+N | -N | dir]";
  if (command->next > 2) {
    fprintf(stderr, "%s\n", help_msg);
  } else if (command->next == 1) { // just pushd, switch the top two dirs in dir_stack
    if (!dir_stack->next->next) // dir_stack has only one element
      fprintf(stderr, "shell: pushd: no other directory\n");
    else {
      l_node *dir;
      dir = dir_stack->next;
      dir_stack->next = dir->next;
      dir->next = dir_stack->next->next;
      dir_stack->next = dir;
      sh_chdir((char *)dir->val);
      print_dir_stack(stdout);
    }
  } else {
    if (sh_chdir(command->argv[1]) != 0) {
      fprintf(stderr, "shell: pushd: %s: %s\n", command->argv[1], strerror(errno));
    } else {
      push(&dir_stack, sh_getcwd());
      print_dir_stack(stdout);
    }
  }
}
void popd(cmd *command)
{
  int i;
  int pos = 0;
  int err;
  int dir_chang_flag = 1;
  int from_end = 0;
  int f[] = {0, 0}; // indicate whether two options have occurred
  char *dir;
  static char *help_msg = "popd: usage: popd [-n] [+N | -N]";
  if (!dir_stack->next->next) {
    fprintf(stderr, "shell: popd: directory stack empty\n");
    return;
  }
  if (command->next > 2) {
    fprintf(stderr, "%s\n", help_msg);
  } else {
    for (i = 1; i < command->next; ++i) { // process options
      if (err) {
        fprintf(stderr, "%s\n", help_msg);
        return;
      }
      if (command->argv[i][0] == '-') { // -n or -N
        if (strcmp(command->argv[i], "-n") == 0) {
          if (f[0] == 1)
            err = 1;
          else {
            f[0] = 1;
            dir_chang_flag = 0;
          }
          continue;
        }
        pos = sh_strtoi(command->argv[i], &err);
        if (err) {
          fprintf(stderr, "shell: popd: %s: not a valid number\n%s\n", &command->argv[i][1], help_msg);
          return;
        }
        if (f[1])
          err = 1;
        else {
          f[1] = 1;
          from_end = 1;
        }
      } else if (command->argv[1][0] == '+') { // +N
        pos = sh_strtoi(command->argv[i], &err);
        if (err) {
          fprintf(stderr, "shell: popd: %s: not a valid number\n%s\n", &command->argv[i][1], help_msg);
          return;
        }
        if (f[1])
          err = 1;
        else {
          f[1] = 1;
          from_end = 0;
        }
      } else {
        fprintf(stderr, "shell: popd: %s: unknown option\n%s\n", command->argv[i], help_msg);
        return;
      }
    }
    dir = (char *)l_remove(&dir_stack, (pos<0 ? -pos : pos), from_end);
    if (!dir) {
      fprintf(stderr, "shell: popd: directory stack index out of range\n");
      return;
    }
    if (dir_chang_flag)
      sh_chdir((char *)dir_stack->next->val);
    free(dir);
    print_dir_stack(stdout);
  }
}
