#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "type.h"

void cmd_init(cmd *cmdp)
{
  static int default_size = 5;
  int i;
  cmdp->size = default_size;
  cmdp->argv = malloc(cmdp->size * sizeof(char *));
  cmdp->next = 0;
  cmdp->rd_size = default_size;
  cmdp->rds = malloc(cmdp->rd_size * sizeof(redirection *));
  cmdp->rd_next = 0;
  cmdp->bg = 0;
  cmdp->cmd_next = NULL;
  cmdp->p_len = 1;
  for (i = 0; i < cmdp->size; ++i) {
    cmdp->argv[i] = NULL;
    cmdp->rds[i] = NULL;
  }
}

void cmd_addarg(cmd *cmdp, const char *arg)
{
  int i;
  if (cmdp->next == cmdp->size - 1) {
    cmdp->size *= 2;
    cmdp->argv = realloc(cmdp->argv, cmdp->size * sizeof(char *));
    for (i = cmdp->next; i < cmdp->size; ++i)
      cmdp->argv[i] = NULL;
  }
  cmdp->argv[cmdp->next] = malloc((strlen(arg)+1) * sizeof(char));
  strcpy(cmdp->argv[cmdp->next++], arg);
}

void cmd_redirect(cmd *cmdp, int fd, elementType type, const char *path)
{
  int i;
  redirectMode mode;
  switch (type) {
  case IN_RED: fd = STDIN_FILENO; mode = RD; break;
  case OUT_RED: fd = STDOUT_FILENO; mode = WR; break;
  case OUT_APPEND: fd = STDOUT_FILENO; mode = WR_APPEND; break;
  case OUT_ERR_RED: fd = -1; mode = WR; break;
  case OUT_ERR_APPEND: fd = -1; mode = WR_APPEND; break;
  case FD_IN_RED: mode = RD; break;
  case FD_OUT_RED: mode = WR; break;
  case FD_APPEND: mode = WR_APPEND; break;
  default: fprintf(stderr, "shell: Unknown redirection type\n"); exit(127);
  }
  if (cmdp->rd_next == cmdp->rd_size) {
    cmdp->size *= 2;
    cmdp->rds = realloc(cmdp->rds, cmdp->rd_size * sizeof(redirection *));
    for (i = cmdp->rd_next; i < cmdp->rd_size; ++i)
      cmdp->rds[i] = NULL;
  }
  cmdp->rds[cmdp->rd_next] = malloc(sizeof(redirection));
  cmdp->rds[cmdp->rd_next]->fd = fd;
  cmdp->rds[cmdp->rd_next]->filenam = malloc(strlen(path) * sizeof(char));
  strcpy(cmdp->rds[cmdp->rd_next]->filenam, path);
  cmdp->rds[cmdp->rd_next]->mode = mode;
  cmdp->rd_next++;
}

void cmd_set_bg(cmd *cmdp)
{
  cmdp->bg = 1;
}

void cmd_dealloc(cmd *cmdp)
{
  int i;
  cmd *delp;
  if (cmdp->cmd_next)
    cmd_dealloc(cmdp->cmd_next);
  for (i = 0; i < cmdp->next; ++i) {
    free(cmdp->argv[i]);
  }
  for (i = 0; i < cmdp->rd_next; ++i) {
    free(cmdp->rds[i]->filenam);
    free(cmdp->rds[i]);
  }
  free(cmdp->argv);
  free(cmdp->rds);
  free(cmdp);
}

void cmd_add_pcmd(cmd **cmdp)
{
  cmd *tmpp = malloc(sizeof(cmd));
  cmd_init(tmpp);
  tmpp->cmd_next = *cmdp;
  *cmdp = tmpp;
  (*cmdp)->p_len = (*cmdp)->cmd_next->p_len + 1;
}
