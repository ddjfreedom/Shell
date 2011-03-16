#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lex.yy.h"
#include "parse.h"

int get_fd(const char *input, elementType type);

int parse(const char *input, cmd *cmds[], int size)
{
  elementType type;
  int cmd_count = -1, fd;
  YY_BUFFER_STATE buf = yy_scan_string(input);
  while ((type = yylex()) != 0) {
    if (cmd_count == -1) cmd_count = 0;
    switch (type) {
    case SEMICOLON:
      cmd_count++; break;
    case PIPE:
      cmd_add_pcmd(&cmds[cmd_count]); break;
    case STRING:
      cmd_addarg(cmds[cmd_count], yytext); break;
    case IN_RED:
      yylex();
      cmd_redirect(cmds[cmd_count], STDIN_FILENO, RD, yytext); break;
    case OUT_RED:
      yylex();
      cmd_redirect(cmds[cmd_count], STDOUT_FILENO, WR, yytext); break;
    case OUT_APPEND:
      yylex();
      cmd_redirect(cmds[cmd_count], STDOUT_FILENO, WR_APPEND, yytext); break;
    case OUT_ERR_RED:
      yylex();
      cmd_redirect(cmds[cmd_count], -1, WR, yytext);
      break;
    case OUT_ERR_APPEND:
      yylex();
      cmd_redirect(cmds[cmd_count], -1, WR_APPEND, yytext);
      break;
    case FD_IN_RED:
      fd = get_fd(yytext, FD_IN_RED);
      yylex();
      cmd_redirect(cmds[cmd_count], fd, RD, yytext); break;
    case FD_OUT_RED:
      fd = get_fd(yytext, FD_OUT_RED);
      yylex();
      cmd_redirect(cmds[cmd_count], fd, WR, yytext); break;
    case FD_APPEND:
      fd = get_fd(yytext, FD_APPEND);
      yylex();
      cmd_redirect(cmds[cmd_count], fd, WR_APPEND, yytext); break;
    case BACKGROUND:
      cmd_set_bg(cmds[cmd_count]); break;
    default:
      fprintf(stderr, "Unknown type: %d, %s\n", type, yytext);
    }
  }
  yy_delete_buffer(buf);
  return cmd_count + 1;
}

int get_fd(const char *input, elementType type)
{
  int fd;
  char *ptr, *loc;
  if (type == FD_IN_RED)
    loc = strchr(input, '<');
  else
    loc = strchr(input, '>');
  fd = strtol(input, &ptr, 10);
  if (ptr == loc)
    return fd;
  fprintf(stderr, "shell: Bad file descriptor\n");
  exit(127);
}

