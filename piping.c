#include "piping.h"
#include "scanner.h"


/*  Creates pipes, 2 for each counted pipe char (count) */
pipe_list_t *create_pipes(int count) {
    pipe_list_t *pipes = malloc(sizeof(pipe_list_t));
    if(!alloc_check(pipes)) {
        return 0;
    }

    if(count > 0) {
        pipes->pipes = malloc(2 * count * sizeof(int *));
        if(!alloc_check(pipes->pipes)) {
            return 0;
        }

        for(int i = 0; i < count; i++ ) {
            (pipes->pipes)[i] = malloc(2 * sizeof(int *));
            if(!alloc_check(pipes->pipes[i])) {
                free(pipes->pipes);
                free(pipes);
                return 0;
            }
            if(pipe(pipes->pipes[i]) < 0) {
                perror("Couldn't create a pipe");
            }
        }
    }

    pipes->length = count;
    pipes->curr = 0;

    return pipes;
}

/*  Frees all pipes */
void cleanup_pipes(pipe_list_t *pipes) {
    for(int i = 0; i < pipes->length; i++) {
        free(pipes->pipes[i]);
    } 
    if(pipes->length > 0) {
       free(pipes->pipes); 
    }
    free(pipes);
}

/*  Returns the current pipe */
int *curr_pipe(pipe_list_t *pipes) {
    return pipes->pipes[pipes->curr];
}

/*  Returns the previous pipe */
int *prev_pipe(pipe_list_t *pipes) {
    return pipes->pipes[pipes->curr-1];
}
