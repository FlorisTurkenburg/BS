#ifndef scanner.h
#define scanner.h

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
void executeCommand(unsigned char *commandStr);
void parseCommand(unsigned char *commandStr);
int scanLine(FILE *fd);

#endif
