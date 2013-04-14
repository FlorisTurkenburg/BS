/*
 *
 * NAME: SAMUEL NORBURY, FLORIS TURKENBURG.
 * STUDENTID: 10346643, 10419667.
 * DATE: 15-03-2013.
 *
 * files: scanner.c, scanner.h, piping.c, piping.h.
 *
 * This file contains a minimalist shell.
 *
 */

#include "piping.h"
#include "scanner.h"

/* Function to create and allocated a pipe list. */
pipes_list *make_pipes(int count) {
    pipes_list *pipes = malloc(sizeof(pipes_list));
    if(!check_allocation(pipes)) {
        return 0;
    }

    if(count > 0) {
        pipes->pipes = malloc(2 * count * sizeof(int *));
        if(!check_allocation(pipes->pipes)) {
            return 0;
        }

        for(int i = 0; i < count; i++ ) {
            (pipes->pipes)[i] = malloc(2 * sizeof(int *));
            if(!check_allocation(pipes->pipes[i])) {
                free(pipes->pipes);
                free(pipes);
                return 0;
            }
            if(pipe(pipes->pipes[i]) < 0) {
                perror("Pipe create failed");
            }
        }
    }

    pipes->length = count;
    pipes->current = 0;

    return pipes;
}

/* Free the memory of the pipe. */
void free_pipes(pipes_list *pipes) {
    for(int i = 0; i < pipes->length; i++) {
        free(pipes->pipes[i]);
    }
    if(pipes->length > 0) {
       free(pipes->pipes);
    }
    free(pipes);
}

/* Return the number of the curent pipe. */
int *current_pipe(pipes_list *pipes) {
    return pipes->pipes[pipes->current];
}

/* Return the number of the previous pipe. */
int *previous_pipe(pipes_list *pipes) {
    return pipes->pipes[pipes->current-1];
}