%{
#include "type.h"
#include "stdio.h"
#include "string.h"
%}  

%option noyywrap
WS [ \t\n]
VALIDCHS [[:alnum:]+\-/.&]
%%

{VALIDCHS}+ { return STRING; }
{WS}+ /* eat up white spaces */
; { return SEMICOLON; }
\| { return PIPE; }
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
