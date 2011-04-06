#include <stdio.h>
#include <string.h>
#include "builtin.h"

static char *short_help[] = {
  "help [pattern ...]",
  "cd [dir]",
  "pwd",
  "dirs",
  "pushd [dir]",
  "popd [-n] [+N | -N]",
  "jobs",
  "fg [job_spec]",
  "bg [job_spec]",
  "kill pid | job_spec ...",
  "exit [n]"
};

static char *long_help[] = {
  "help: help [pattern ...]\n"
  "    Display information about builtin commands.\n\n"
  "    Displays brief summaries of builtin commands.  If pattern is specified,\n"
  "    gives detailed help on all commands matching pattern, otherwise the list\n"
  "    of help topics is printed.\n"
  "    Argument:\n"
  "      pattern    Pattern specifiying a help topic\n",

  "cd: cd [dir]\n"
  "    Change the shell working directory.\n\n"
  "    Change the current directory to dir. The default dir is the value of the\n"
  "    HOME shell variable.\n"
  "    If any error occurs, the current directory remains unchanged.\n",
  
  "pwd: pwd\n"
  "    Print the absolute path of the current directory to standard output.\n",

  "dirs: dirs\n"
  "    Display directory stack.\n\n"
  "    Display the list of currently remembered directories. Directories find\n"
  "    their way onto the list with the `pushd' command; you can get back up\n"
  "    through the list with the `popd' command.\n",

  "pushd: pushd [dir]\n"
  "    Add directories to stack.\n\n"
  "    Adds directory dir to the top of the directory stack. With no arguments,\n"
  "    exchanges the top two directories.\n",

  "popd: popd [-n] [+N | -N]\n"
  "    Remove directories from stack.\n\n"
  "    Removes entries from the directory stack. With no arguments, removes the\n"
  "    top directory from the stack, and changes to the new top directory.\n\n"
  "    Options:\n"
  "      -n    Suppresses the normal change of directory when removing\n"
  "        directories from the stack, so only the stack is manipulated.\n"
  "    Arguments:\n"
  "      +N    Removes the Nth entry counting from the left of the list shown\n"
  "        by `dirs', starting with zero.  For example: `popd +0' removes the\n"
  "        first directory, `popd +1' the second.\n\n"
  "      -N    Removes the Nth entry counting from the right of the list shown\n"
  "        by `dirs', starting with zero.  For example: `popd -0' removes the\n"
  "        last directory, `popd -1' the next to last.\n",

  "jobs: jobs\n"
  "    Display status of jobs.\n\n"
  "    Lists the status of all active jobs\n",

  "fg: fg [job_spec]\n"
  "    Move job to the foreground.\n\n"
  "    Place the job identified by job_spec in the foreground, making it the\n"
  "    current job.  If job_spec is not present, the shell's notion of the\n"
  "    current job is used.\n",
  
  "bg: bg [job_spec]\n"
  "    Move job to the background.\n\n"
  "    Place the job identified by job_spec in the background, as if it\n"
  "    had been started with `&'.  If job_spec is not present, the shell's\n"
  "    notion of the current job is used.\n",

  "kill: kill pid | job_spec ...\n"
  "    Send signal SIGTERM to a job.\n\n"
  "    Send signal SIGTERM to the process identified by pid or job_spec.\n",

  "exit: exit [n]\n"
  "    Exit the shell.\n\n"
  "    Exits the shell with a status of n. If n is omitted, the exit status\n"
  "    is 0.\n"
};

void print_short_help(FILE *out, char *cmdname)
{
  int i;
  for (i = 0; i < BUILTIN_N; ++i)
    if (!strcmp(cmdname, builtin_list[i]))
      break;
  fprintf(out, "%s: usage: %s\n", cmdname, short_help[i]);
}

void sh_help(cmd *command)
{
  int i, j, success = 0;
  if (command->next == 1) { // just help
    for (i = 0; i < BUILTIN_N; ++i)
      printf("%s\n", short_help[i]);
  } else {
    for (i = 1; i < command->next; ++i) {
      for ( j = 0; j < BUILTIN_N; ++j)
        if (!strcmp(builtin_list[j], command->argv[i]))
          break;
      if (j < BUILTIN_N) {
        printf("%s", long_help[j]);
        success = 1;
      }
    }
    if (!success) {
      fprintf(stderr,
              "shell: help: no help topics match `%s'."
              "Try `help help' or `man -k %s' or `info %s'\n",
              command->argv[i-1],
              command->argv[i-1],
              command->argv[i-1]);
    }
  }
}
