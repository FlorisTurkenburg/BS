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

#ifndef piping_h
#define piping_h

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int **pipes;
    int current;
    int length;
} pipes_list;

pipes_list *make_pipes(int count);
void free_pipes(pipes_list *pipes);
int *current_pipe(pipes_list *pipes);
int *previous_pipe(pipes_list *pipes);

#endif
