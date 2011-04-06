#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include "builtin.h"
#include "type.h"

static job *jobs = NULL;
static job *tail = NULL;
static job_count = 1;
static char **jobctl_msgs;
static int jobctl_msg_size;
static int jobctl_msg_count = 0;

void jobctl_init()
{
  int i;
  jobctl_msg_size = 5;
  jobs = malloc(sizeof(job));
  jobs->n = 0;
  jobs->pid = 0;
  jobs->status = 0;
  jobs->cmdname = NULL;
  jobs->next = NULL;
  tail = jobs;
  jobctl_msgs = malloc(jobctl_msg_size * sizeof(char *));
  for (i = 0; i < jobctl_msg_size; ++i)
    jobctl_msgs[i] = NULL;
}
void jobctl_add_job(pid_t pid, char *cmdname, int status, int bg)
{
  job *ptr = jobs;
  while (ptr->next && ptr->next->pid != pid)
    ptr = ptr->next;
  if (ptr->next) {
    tail->next = ptr->next;
    ptr->next = ptr->next->next;
    tail = tail->next;
    tail->next = NULL;
    if (strcmp(tail->cmdname, cmdname)) {
      free(tail->cmdname);
      tail->cmdname = sh_strcpy(cmdname);
    }
    tail->status = status;
    return;
  }
  tail->next = malloc(sizeof(job));
  tail = tail->next;
  tail->n = job_count++;
  tail->pid = pid;
  tail->cmdname = malloc((strlen(cmdname)+1) * sizeof(char));
  strcpy(tail->cmdname, cmdname);
  tail->next = NULL;
  tail->status = status;
  tail->bg = bg;
}
void jobctl_rm_job(pid_t pid, int status)
{
  job *ptr = jobs;
  job *del;
  char *stat;
  while (ptr->next && ptr->next->pid != pid)
    ptr = ptr->next;
  if (!ptr->next) return;
  del = ptr->next;
  ptr->next = del->next;
  tail = ptr;
  while (tail->next) tail = tail->next;
  if (del->bg || del->status == JOBCTL_STP) {
    switch (status) {
    case SIGTERM: stat = "Terminated"; break;
    case SIGKILL: stat = "Killed"; break;
    case SIGABRT: stat = "Abort trap"; break;
    default: stat = "Done"; break;
    }
    fprintf(stderr, "rm job %d\n", jobctl_msg_count);
    jobctl_msgs[jobctl_msg_count] = malloc((strlen(del->cmdname)+30) * sizeof(char));
    sprintf(jobctl_msgs[jobctl_msg_count++], "[%d]%c %s\t\t%s",
            del->n, (del == tail ? '+' : ' '), stat, del->cmdname);
    jobctl_print_msgs();
  }
  free(del->cmdname);
  free(del);
}
void jobctl_print_msgs()
{
  int i;
  for (i = 0; i < jobctl_msg_count; ++i) {
    printf("%s\n", jobctl_msgs[i]);
    free(jobctl_msgs[i]);
  }
  jobctl_msg_count = 0;
}
void sh_jobs(cmd *command)
{
  job *ptr = jobs->next;
  while (ptr) {
    if (jobctl_msg_count == jobctl_msg_size)
      enlarge_ptr(jobctl_msgs, &jobctl_msg_size, sizeof(char *));
    jobctl_msgs[jobctl_msg_count] = malloc((strlen(ptr->cmdname)+30) * sizeof(char));
    sprintf(jobctl_msgs[jobctl_msg_count],
            "[%d]%c %s\t\t%s",
            ptr->n,
            (ptr == tail ? '+' : ' '),
            (ptr->status == JOBCTL_RUN ? "Running" : "Stopped"),
            ptr->cmdname);
    jobctl_msg_count++;
    ptr = ptr->next;
  }
}
void fg(cmd *command)
{
  int jobnum, err;
  job *ptr;
  pid_t pid;
  char *cmdname, *chptr, *chptr2;
  static char *help_msg = "fg: usage: fg [job_spec]";
  //fprintf(stderr, "fg\n");
  if (command->next == 1) // just fg
    ptr = tail;
  else {
    if (command->argv[1][0] == '%')
      jobnum = sh_strtoi(command->argv[1]+1, &err);
    else
      jobnum = sh_strtoi(command->argv[1], &err);
    if (err != 0) { // error occurred
      fprintf(stderr, "shell: fg: %s: invalid option\n%s\n", command->argv[1], help_msg);
      return;
    }
    ptr = jobs->next;
    while (ptr && ptr->n != jobnum)
      ptr = ptr->next;
    if (!ptr) {
      fprintf(stderr, "shell: fg: %s: no such job\n", command->argv[1]);
      return;
    }
  }
  pid = ptr->pid;
  ptr->bg = 0;
  cmdname = malloc((strlen(ptr->cmdname)+1) * sizeof(char));
  strcpy(cmdname, ptr->cmdname);
  chptr = strrchr(cmdname, '&');
  if (chptr) {
    chptr2 = chptr + 1;
    while (*chptr2 == ' ')
      chptr2++;
    if (*chptr2)
      chptr = NULL;
  }
  if (chptr) *chptr = '\0';
  kill(pid, SIGCONT);
  waitcmd(pid, cmdname);
  free(cmdname);
}
void bg(cmd *command)
{
  int jobnum, err;
  job *ptr;
  char *chptr;
  static char *help_msg = "bg: usage: bg [job_spec]";
//  fprintf(stderr, "bg\n");
  if (command->next == 1) // just bg
    ptr = tail;
  else {
    if (command->argv[1][0] == '%')
      jobnum = sh_strtoi(command->argv[1]+1, &err);
    else
      jobnum = sh_strtoi(command->argv[1], &err);
    if (err != 0) { // error occurred
      fprintf(stderr, "shell: bg: %s: invalid option\n%s\n", command->argv[1], help_msg);
      return;
    }
    ptr = jobs->next;
    while (ptr && ptr->n != jobnum)
      ptr = ptr->next;
    if (!ptr) {
      fprintf(stderr, "shell: bg: %s: no such job\n", command->argv[1]);
      return;
    }
  }
  ptr->bg = 1;
  ptr->cmdname = realloc(ptr->cmdname, (strlen(ptr->cmdname)+3) * sizeof(char));
  chptr = ptr->cmdname;
  while (*chptr) chptr++;
  *chptr = ' ';
  *(chptr + 1) = '&';
  *(chptr + 2) = '\0';
  ptr->status = JOBCTL_RUN;
  kill(ptr->pid, SIGCONT);
}
void sh_kill(cmd *command)
{
  int i, j, err;
  int sig = SIGTERM;
  int id;
  job *ptr;
  if (command->next == 1) {
    // TODO: print error message
  } else if (strcmp(command->argv[1], "-l") == 0) { // kill -l [arg]
    
  } else { // kill [-s signame | -signame | -signum] pid | jobspec ...
    //if (command->argv[1][0] == '-')
    i = 1;
    while (i < command->next) {
      err = 0;
      if (command->argv[i][0] == '%') {
        id = sh_strtoi(command->argv[i]+1, &err);
        // if (err) // error handling
        ptr = jobs->next;
        while (ptr && ptr->n != id)
          ptr = ptr->next;
        if (ptr) {
          err = kill(ptr->pid, sig);
          if (err) {
            fprintf(stderr, "shell: kill: %s\n", strerror(errno));
            return;
          }
        } else {
          fprintf(stderr, "shell: kill: %%%d: no such job\n", id);
          return;
        }
      } else {
        id = sh_strtoi(command->argv[i], &err);
        //printf("pid: %d\n", id);
        err = kill(id, sig);
        if (err) {
          fprintf(stderr, "shell: kill: %d: %s\n", id, strerror(errno));
          return;
        }
      }
      i++;
    }
  }
}
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
  if (jobs->next) {
    fprintf(stderr, "shell: exit: There are stopped jobs\n");
    return;
  }
  exit(rt_val);
}
