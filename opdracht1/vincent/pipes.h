#ifndef PIPE_H
#define PIPE_H

typedef struct {
    int **pipes;
    int curr;
    int length;
} pipe_list_t;

pipe_list_t *create_pipes(int count);
void cleanup_pipes(pipe_list_t *pipes);
int *curr_pipe(pipe_list_t *pipes);
int *prev_pipe(pipe_list_t *pipes);

#endif