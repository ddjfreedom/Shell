#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include "builtin.h"

int BUILTIN_N = 8;
const char *builtin_list[] = {
  "cd",
  "pwd",
  "dirs",
  "pushd",
  "popd",
  "jobs",
  "kill",
  "exit"
};
builtin_cmd *builtins[] = {
  cd, 
  pwd,
  dirs,
  pushd,
  popd,
  jobs,
  sh_kill,
  sh_exit
};

extern void dir_init();
extern void jobctl_init();
static void sig_chld(int signo);
void sh_init()
{
  dir_init();
  jobctl_init();
  signal(SIGCHLD, sig_chld);
}
static void sig_chld(int signo)
{
  pid_t pid;
  while ((pid = waitpid(0, NULL, WNOHANG)) > 0) {
    //fprintf(stderr, "sig_chld, pid: %d\n", pid);
    jobctl_rm_bg(pid);
  }
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
