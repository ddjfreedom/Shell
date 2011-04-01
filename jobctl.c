#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "builtin.h"
#include "type.h"

static bg_cmd **bgcmds = NULL;
static int size = 0;
static int next = 0;
static char **jobctl_msgs;
static int jobctl_msg_size;
static int jobctl_msg_count = 0;

void jobctl_init()
{
  int i;
  size = jobctl_msg_size = 5;
  bgcmds = malloc(size * sizeof(bg_cmd *));
  jobctl_msgs = malloc(jobctl_msg_size * sizeof(char *));
  for (i = 0; i < size; ++i) {
    bgcmds[i] = NULL;
    jobctl_msgs[i] = NULL;
  }
}
void jobctl_add_bg(pid_t pid, char *cmdname)
{
  if (next == size)
    enlarge_ptr(bgcmds, &size, sizeof(bg_cmd *));
  bgcmds[next] = malloc(sizeof(bg_cmd));
  bgcmds[next]->pid = pid;
  bgcmds[next]->n = next+1;
  bgcmds[next]->cmdname = malloc((strlen(cmdname)+1) * sizeof(char));
  strcpy(bgcmds[next]->cmdname, cmdname);
  jobctl_msgs[jobctl_msg_count] = malloc(20 * sizeof(char));
  sprintf(jobctl_msgs[jobctl_msg_count], "[%d]  %d", bgcmds[next]->n, pid);
  jobctl_msg_count++;
  next++;
}
void jobctl_rm_bg(pid_t pid)
{
  int i;
  bg_cmd *del;
  char *msg;
  for (i = 0; i < next; ++i)
    if (bgcmds[i]->pid == pid) break;
  if (i == next) return;
  del = bgcmds[i];
  msg = malloc((strlen(del->cmdname)+20) * sizeof(char));
  sprintf(msg, "[%d]%c Done\t\t%s", del->n, (i==next-1 ? '+' : ' '),
          del->cmdname);
  for (; i < next; ++i)
    bgcmds[i] = bgcmds[i+1];
  next--;
  free(del->cmdname);
  free(del);
  if (jobctl_msg_count == jobctl_msg_size)
    enlarge_ptr(jobctl_msgs, &jobctl_msg_size, sizeof(char *));
  jobctl_msgs[jobctl_msg_count++] = msg;
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
void jobs(cmd *command)
{
  int i;
  for (i = 0; i < next; ++i) {
    if (jobctl_msg_count == jobctl_msg_size)
      enlarge_ptr(jobctl_msgs, &jobctl_msg_size, sizeof(char *));
    jobctl_msgs[jobctl_msg_count] = malloc((strlen(bgcmds[i]->cmdname)+20) * sizeof(char));
    sprintf(jobctl_msgs[jobctl_msg_count], "[%d]%c\t\t\t%s", bgcmds[i]->n,
            (i==next-1 ? '+' : ' '), bgcmds[i]->cmdname);
    jobctl_msg_count++;
  }
  jobctl_print_msgs();
}
void sh_kill(cmd *command)
{
  int i, j, err;
  int sig = SIGTERM;
  int id;
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
        if (id-1 < next) {
          err = kill(bgcmds[id-1]->pid, sig);
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
  //TODO: handle background processes
  exit(rt_val);
}
