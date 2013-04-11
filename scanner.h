#ifndef scanner_h
#define scanner_h

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

typedef int builtinFun(char *command);

struct builtin_Func {
    builtinFun *fun;
    char name[32];
};

void run_shell();
int check_allocation(void *);
int do_exit(char *command);
int do_cd(char *command);
int do_source(char *command);
void free_array(char ***array);
char *trimwhitespace(char *str);
void executeCommand(char *cmdline, pipes_list *pipes);
void parseCommand(unsigned char *command_string);
int scanLine(FILE *fd);
void signal_handler(int s);
int split_string(char *string, char ***array, char split_character);

#endif
