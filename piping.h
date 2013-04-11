#ifndef piping.h
#define piping.h

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int **pipes;
    int curr;
    int length;
} pipes_list;

pipes_list *create_pipes(int count);
void cleanup_pipes(pipes_list *pipes);
int *curr_pipe(pipes_list *pipes);
int *prev_pipe(pipes_list *pipes);

#endif
