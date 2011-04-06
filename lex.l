%{
#include "type.h"
#include "stdio.h"
#include "string.h"
%}  

%option noyywrap
WS [ \t\n]
VALIDCHS [[:alnum:]+\-/.~]
%%

  int str_count = 0;

({VALIDCHS}|\\\ |\\&)+|(&[[:digit:]]+)|(\%[[:digit:]]+) {
  if (str_count++)
      return STRING;
    else
      return CMD;
  }
{WS}+ /* eat up white spaces */
; { str_count = 0; return SEMICOLON; }
\| { str_count = 0; return PIPE; }
  /* IO redirection */
\< { return IN_RED; }
\> { return OUT_RED; }
\>\> { return OUT_APPEND; }
&\> { return OUT_ERR_RED; }
&\>\> { return OUT_ERR_APPEND; }
[[:digit:]]+\> { return FD_OUT_RED; }
[[:digit:]]+\>\> { return FD_APPEND; }
[[:digit:]]+\< { return FD_IN_RED; }
  /* background */
&{WS}*$ { return BACKGROUND; }

. { fprintf(stderr, "lex: Unknown char: %s\n", yytext); }
%%

/*
#define MAXSIZE 200
int main(int argc, const char *argv[])
{
  enum elementType type;
  char in[MAXSIZE];
  while (fgets(in, MAXSIZE, stdin) != NULL) {
    YY_BUFFER_STATE buf = yy_scan_string(in);
    while ((type = yylex()) != 0) {
      printf("%d: %s\n", type, yytext);
    }
    yy_delete_buffer(buf);
  }
  return 0;
}
*/
