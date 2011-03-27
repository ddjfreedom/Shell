#ifndef TYPE_H
#define TYPE_H

typedef enum elementType {
  CMD = 1,
  STRING,
  SEMICOLON,
  PIPE,
  IN_RED,
  OUT_RED,
  OUT_APPEND,
  OUT_ERR_RED,
  OUT_ERR_APPEND,
  FD_IN_RED,
  FD_OUT_RED,
  FD_APPEND,
  BACKGROUND
} elementType;

typedef enum redirectMode {
  RD = 1,
  WR,
  WR_APPEND
  // RDWR
//  RDWR_APPEND
} redirectMode;

typedef struct redirection {
  int fd; // -1 means redirect both stdout and stderr
  char *filenam;
  redirectMode mode;
} redirection;

typedef struct cmd {
  char **argv;
  int size;
  int next; // the index of the first unused element of argv
  redirection **rds; // redirections
  int rd_size; // total number of redirections
  int rd_next; // index of the first unused element of rds
  int bg; //non-zero means to run cmd in the background
  // in situations like cmd1 | cmd2, cmd_next of cmd2 points to cmd1
  struct cmd *cmd_next;
  int p_len; // number of commands in pipeline
} cmd;

void cmd_init(cmd *cmdp);
void cmd_addarg(cmd *cmdp, const char *arg);
void cmd_redirect(cmd *cmdp, int fd, elementType mode, const char *path);
void cmd_set_bg(cmd *cmdp);
void cmd_dealloc(cmd *cmdp);
void cmd_add_pcmd(cmd **cmdp);

// alias alias="name"
typedef struct alias_t {
  char *name;
  char *action;
  struct alias_t *next;
} alias_t;

#endif
