#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "builtin.h"

int BUILTIN_N = 11;
const char *builtin_list[] = {
  "help",
  "cd",
  "pwd",
  "dirs",
  "pushd",
  "popd",
  "jobs",
  "fg",
  "bg",
  "kill",
  "exit"
};
builtin_cmd *builtins[] = {
  sh_help,
  cd, 
  pwd,
  dirs,
  pushd,
  popd,
  sh_jobs,
  fg,
  bg,
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
  signal(SIGTSTP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
}
static void sig_chld(int signo)
{
  pid_t pid;
  int stat;
  while ((pid = waitpid(0, &stat, WNOHANG)) > 0) {
    //fprintf(stderr, "sig_chld, pid: %d\n", pid);
    if (WIFEXITED(stat))
      jobctl_rm_job(pid, 0);
    else if (WIFSIGNALED(stat)) {
      stat = WTERMSIG(stat);
      jobctl_rm_job(pid, stat);
    } else {
      //fprintf(stderr, "sig_chld: not exited, not signaled\n");
      jobctl_rm_job(pid, 0);
    }
  }
}
void waitcmd(pid_t pid, char *cmdname)
{
  int status;
  waitpid(pid, &status, WUNTRACED);
  if (WIFSTOPPED(status)) {
    //fprintf(stderr, "stoped pid: %d\n", pid);
    jobctl_add_job(pid, cmdname, JOBCTL_STP, 0);
  }// else
    //fprintf(stderr, "done waitpid: %d\n", pid);
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
char *sh_strcpy(char *str)
{
  char *res = malloc((strlen(str)+1) * sizeof(char));
  strcpy(res, str);
  return res;
}
