#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "redirect.h"

int redirect(cmd *command)
{
  int i;
  char *ptr;
  int fd;
  for (i = 0; i < command->rd_next; ++i) {
    if (command->rds[i]->filenam[0] == '&') {
      fd = strtol(&command->rds[i]->filenam[1], &ptr, 10);
      if (!*ptr) {
        if (dup2(fd, command->rds[i]->fd) < 0) {
          // error handling
          fprintf(stderr, "shell: bad file descriptor: %d\n", fd);
          exit(127);
        }
        continue;
      }
    }
    switch (command->rds[i]->mode) {
    case RD:
      fd = open(command->rds[i]->filenam, O_RDONLY);
      break;
    case WR:
      fd = open(command->rds[i]->filenam, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      break;
    case WR_APPEND:
      fd = open(command->rds[i]->filenam, O_WRONLY | O_CREAT | O_APPEND, 0666);
      break;
    default:
      fprintf(stderr, "Unknown redirect mode: %d\n", command->rds[i]->mode);
      break;
    }
    if (fd < 0) {
      fprintf(stderr, "shell can't open file: %s\n", command->rds[i]->filenam);
      exit(127);
    }
    if (fd != command->rds[i]->fd) {
      dup2(fd, command->rds[i]->fd);
      close(fd);
    }
  }
}
