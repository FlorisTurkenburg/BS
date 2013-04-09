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

void run_shell();
int check_allocation(void *);

#endif
