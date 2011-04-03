#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <readline/readline.h>
#include "type.h"
#include "parse.h"
#include "redirect.h"
#include "builtin.h"

#define INIT_SIZE 5
#define BUFMAXSIZE 200

int dir_changed = 1;
char *PS1 = "\\u:\\w$ ";
char *buf;
int cmds_init(cmd **cmds, int size);
void print(cmd **cmds, int size);
int exec_cmd(cmd *command);
char *getprompt();

int main(int argc, char *argv[])
{
  int count, i, tmp, j;
  int builtin_f;
  cmd **cmds;
  //char *buf = malloc(BUFMAXSIZE * sizeof(char));
  sh_init();
  while (buf = readline(getprompt())) {
    if (*buf)
      add_history(buf);
    cmds = malloc(INIT_SIZE * sizeof(cmd *));
    cmds_init(cmds, INIT_SIZE);
    tmp = strlen(buf);
    realloc(buf, (tmp + 2) * sizeof(char));
    buf[tmp] = '\n'; buf[tmp+1] = '\0';
    count = parse(buf, cmds, INIT_SIZE);
    buf[tmp] = '\0';
    //print(cmds, count);
    if (count != -1)
      for (i = 0; i < count; ++i) {
        builtin_f = 0;
        for (j = 0; j < BUILTIN_N; ++j)
          if (strcmp(cmds[i]->argv[0], builtin_list[j]) == 0) {
            (*builtins[j])(cmds[i]);
            builtin_f = 1;
            break;
          }
        if (!builtin_f)
          exec_cmd(cmds[i]);
      }
    jobctl_print_msgs();
    for (i = 0; i < count; ++i)
      cmd_dealloc(cmds[i]);
    free(buf);
  }
  return 0;
}

int cmds_init(cmd **cmds, int size)
{
  int i;
  for (i = 0; i < size; ++i) {
    cmds[i] = malloc(sizeof(cmd));
    cmd_init(cmds[i]);
  }
  return 0;
}

int exec_cmd(cmd *command)
{
  pid_t *pids = malloc(command->p_len * sizeof(pid_t));
  int status, i;
  int fds[2][2];
  cmd *cmdp;
  status = pipe(fds[1]); // reduce complexity in the following code
  //fprintf(stderr, "p_len: %d\n", command->p_len);
  for (cmdp = command, i = 0; i < command->p_len; cmdp = cmdp->cmd_next, i++) {
    status = pipe(fds[i % 2]);
    if (status < 0) {
      perror("shell: pipe");
      exit(127);
    }
    if ((pids[i] = fork()) < 0) {
      perror("shell: fork");
      exit(127);
    } else if (pids[i] == 0) { // child process
      signal(SIGTSTP, SIG_DFL);
      signal(SIGINT, SIG_DFL);
      signal(SIGCONT, SIG_DFL);
      if (i != command->p_len-1) // not the first command in pipeline, redirect stdin
        dup2(fds[i%2][0], STDIN_FILENO);
      if (i != 0) // not the last command in pipeline, redirect stdout
        dup2(fds[!(i%2)][1], STDOUT_FILENO);
      close(fds[0][0]); close(fds[0][1]);
      close(fds[1][0]); close(fds[1][1]);
      redirect(cmdp);
      //fprintf(stderr, "exec: %d %d %s\n", i, (int)cmdp, cmdp->argv[0]);
      execvp(cmdp->argv[0], cmdp->argv);
      fputs("shell: ", stderr);
      perror(cmdp->argv[0]);
      exit(127);
    } else { // parent process
      close(fds[!(i%2)][0]); close(fds[!(i%2)][1]);
    }
  }
  //printf("pid: %d\n", pids[0]);
  if (command->bg) {
    jobctl_add_job(pids[0], buf, JOBCTL_RUN, 1);
  } else {
    waitcmd(pids[0], buf);
  }
  free(pids);
  return status;
}

char *getprompt()
{
  static char *prompt = NULL;
  static int len = 0;
  static int default_len = 30;
  if (dir_changed) {
    int i, j;
    char *tmp;
    int allocated = 0;
    int path_len = 100;
    dir_changed = 0;
    prompt = realloc(prompt, (len = default_len) * sizeof(char));
    for (i = 0, j = 0; i < strlen(PS1); ++i) {
      if (PS1[i] == '\\') {
        switch (PS1[++i]) {
        case 'u':
          tmp = getlogin(); allocated = 0; break;
        case 'w':
          tmp = sh_getcwd(); allocated = 1; break;
        default:
          fprintf(stderr, "shell: Unknown PS1: %s\n", PS1); exit(127);
        }
        prompt[j] = '\0';
        j += strlen(tmp);
        if (j >= len) {
          len *= 2;
          prompt = realloc(prompt, len * sizeof(char));
        }
        strcat(prompt, tmp);
        if (allocated) free(tmp);
      } else {
        if (j == len-1) {
          len *= 2;
          prompt = realloc(prompt, len * sizeof(char));
        }
        prompt[j++] = PS1[i];
      }
    }
    prompt[j] = '\0';
  }
  return prompt;
}

void print(cmd **cmds, int size)
{
  int i, j;
  cmd *command;
  for (i = 0; i < size; ++i) {
    command = cmds[i];
    while (command) {
      for (j = 0; j < command->next; ++j)
        fprintf(stdout, "%s\n", command->argv[j]);
      for (j = 0; j < command->rd_next; ++j) {
        fprintf(stdout, "%d %d %s\n", command->rds[j]->fd, command->rds[j]->mode,
                command->rds[j]->filenam);
      }
      fprintf(stdout, "bg: %s\n", (command->bg ? "yes" : "no"));
      command = command->cmd_next;
    }
  }
}
