#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "type.h"
#include "parse.h"
#include "redirect.h"

#define INIT_SIZE 5
#define BUFMAXSIZE 200

int cmds_init(cmd **cmds, int size);
void print(cmd **cmds, int size);
int exec_cmd(cmd *command);
int create_pipes(int num, int **fds);
int main(int argc, char *argv[])
{
  int count, i;
  cmd **cmds;
  char *buf = malloc(BUFMAXSIZE * sizeof(char));
  while (fgets(buf, BUFMAXSIZE, stdin) != NULL) {
    cmds = malloc(INIT_SIZE * sizeof(cmd *));
    cmds_init(cmds, INIT_SIZE);
    count = parse(buf, cmds, INIT_SIZE);
    //print(cmds, count);
    for (i = 0; i < count; ++i)
      exec_cmd(cmds[i]);
    for (i = 0; i < count; ++i)
      cmd_dealloc(cmds[i]);
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
      fputs("shell: pipe error\n", stderr);
      exit(127);
    }
    if ((pids[i] = fork()) < 0) {
      fputs("shell: fork error\n", stderr);
      exit(127);
    } else if (pids[i] == 0) { // child process
      if (i != command->p_len-1) // not the first command in pipeline, redirect stdin
        dup2(fds[i%2][0], STDIN_FILENO);
      if (i != 0) // not the last command in pipeline, redirect stdout
        dup2(fds[!(i%2)][1], STDOUT_FILENO);
      close(fds[0][0]); close(fds[0][1]);
      close(fds[1][0]); close(fds[1][1]);
      redirect(cmdp);
      //fprintf(stderr, "exec: %d %d %s\n", i, (int)cmdp, cmdp->argv[0]);
      execvp(cmdp->argv[0], cmdp->argv);
      fprintf(stderr, "Unknown command: %s\n", cmdp->argv[0]);
      exit(127);
    } else {
      close(fds[!(i%2)][0]); close(fds[!(i%2)][1]);
    }
  }
  waitpid(pids[0], &status, 0);
  free(pids);
  /* if ((pid = fork()) < 0) { */
  /*   fputs("fork error\n", stderr); */
  /*   exit(127); */
  /* } else if (pid == 0) { // child process */
  /*   redirect(command); */
  /*   execvp(command->argv[0], command->argv); */
  /*   fprintf(stderr, "Unknown command: %s\n", command->argv[0]); */
  /*   exit(127); */
  /* } else { */
  /*   waitpid(pid, &status, 0); */
  /* } */
  return status;
}

int create_pipes(int num, int **fds)
{
  int i;
  for (i = 0; i < num; ++i)
    if (pipe(fds[i]) < 0)
      return -1;
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
