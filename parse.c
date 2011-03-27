#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lex.yy.h"
#include "parse.h"

int get_fd(const char *input, elementType type);
char *handle_escaped_chs(const char *input);

int parse(const char *input, cmd *cmds[], int size)
{
  elementType type;
  redirectMode redir;
  int cmd_count = -1, fd;
  int valid_pipe = 0;
  char *str;
  YY_BUFFER_STATE buf = yy_scan_string(input);
  while ((type = yylex()) != 0) {
    if (cmd_count == -1) cmd_count = 0;
    fd = -1;
    str = NULL;
    switch (type) {
    case SEMICOLON:
      cmd_count++; valid_pipe = 0; break;
    case PIPE:
      if (!valid_pipe) {
        fprintf(stderr, "shell: invalid pipe\n");
        yy_delete_buffer(buf);
        return -1;
      }
      cmd_add_pcmd(&cmds[cmd_count]); break;
    case CMD:
      valid_pipe = 1;
    case STRING:
      str = handle_escaped_chs(yytext);
      //fprintf(stderr, "%s %s\n", yytext, str);
      cmd_addarg(cmds[cmd_count], str); break;
    case FD_IN_RED: case FD_OUT_RED: case FD_APPEND:
      fd = get_fd(yytext, type);
    case IN_RED: case OUT_RED: case OUT_APPEND: case OUT_ERR_RED: case OUT_ERR_APPEND:
      yylex();
      str = handle_escaped_chs(yytext);
      cmd_redirect(cmds[cmd_count], fd, type, str); break;
    case BACKGROUND:
      cmd_set_bg(cmds[cmd_count]); break;
    default:
      fprintf(stderr, "Unknown type: %d, %s\n", type, yytext);
    }
    free(str);
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

char *handle_escaped_chs(const char *input)
{
  int i, j, error = 0;
  char *tmp = malloc((strlen(input)+1) * sizeof(char));
  for (i = 0, j = 0; j < strlen(input) && !error; ++i, ++j) {
    if (input[j] == '\\') {
      switch (input[++j]) {
      case ' ': case '*': case '?': case '|': case '&':
        tmp[i] = input[j]; break;
      default:
        fprintf(stderr, "shell: Unknown escaped character: \\%c", input[j]);
        error = 1;
        break;
      }
    } else
      tmp[i] = input[j];
  }
  tmp[i] = '\0';
  if (error) {
    free(tmp);
    return NULL;
  }
  return tmp;
}
