/*
*
* NAME: SAMUEL NORBURY, FLORIS TURKENBURG.
* STUDENTID: 10346643, HIERINVULLENFLORIS.
* DATE: 15-03-2013.
*
* files: scanner.c, scanner.h, piping.c, piping.h.
*
* This file contains a minimalist shell.
*
*/

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
#include "piping.h"

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
char *trim_whitespace(char *str);
int execute_command(char *command_line, pipes_list *pipes);
void parse_command(char *command_string);
char *scan_line(FILE *fd);
void signal_handler(int s);
int split_string(char *string, char ***array, char split_character);

#endif
