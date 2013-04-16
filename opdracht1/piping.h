/*
 *
 * NAME: SAMUEL NORBURY, FLORIS TURKENBURG.
 * STUDENTID: 10346643, 10419667.
 * DATE: 15-03-2013.
 *
 * Files: scanner.c, scanner.h, piping.c, piping.h.
 *
 * This file contains the structure definition and the prototypes for a pipe
 * implementation.
 *
 */

#ifndef piping_h
#define piping_h

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* Defintion of a pipe list structure. */
typedef struct {
    int **pipes;
    int current;
    int length;
} pipes_list;

/* Prototypes */
pipes_list *make_pipes(int count);
void free_pipes(pipes_list *pipes);
int *current_pipe(pipes_list *pipes);
int *previous_pipe(pipes_list *pipes);

#endif
