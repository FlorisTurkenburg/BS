#include "piping.h"
#include "scanner.h"


/*  Creates pipes, 2 for each counted pipe char (count) */
pipes_list *create_pipes(int count) {
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
                perror("Couldn't create a pipe");
            }
        }
    }

    pipes->length = count;
    pipes->curr = 0;

    return pipes;
}

/*  Frees all pipes */
void cleanup_pipes(pipes_list *pipes) {
    for(int i = 0; i < pipes->length; i++) {
        free(pipes->pipes[i]);
    } 
    if(pipes->length > 0) {
       free(pipes->pipes); 
    }
    free(pipes);
}

/*  Returns the current pipe */
int *curr_pipe(pipes_list *pipes) {
    return pipes->pipes[pipes->curr];
}

/*  Returns the previous pipe */
int *prev_pipe(pipes_list *pipes) {
    return pipes->pipes[pipes->curr-1];
}
