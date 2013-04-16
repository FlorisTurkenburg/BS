/* "Try to be like the turtle - at ease in your own shell." ~ Bill Copeland 
 *
 * Program:     My own shell
 * Author:      Vincent Hagen
 * Student#:    10305602
 *
 * Description: 
 *      This program is a shell terminal. With piping implemented
 * Buildin functions:
 *      - cd        changed the current working directory
 *      - source    runs a file line by line as commands
 *      - .         alias of source
 *      - exit      Exits the shell terminal
 */

#include "simpleshell.h"
#include "pipes.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>

//all build-in functions
struct buildin_func {
    int (*func)(char *);
    char *name;
};

//build-in function headers
int do_exit(char *cmd);
int do_cd(char *cmd);
int do_source(char *cmd);


//  determines whenever the shell need to be terminated
static int terminated = 0;
static int source_calls = 0;
static char cwd[1024];
static struct buildin_func own_commands[] = {
    {do_exit, "exit"},
    {do_cd, "cd"},
    {do_source, "source"},
    {do_source, "."},
    {NULL, ""}
};

// if alloc failed, return false and terminate shell
int alloc_check(void *ptr) {
    if(!ptr) {
        perror("Couldn't allocate memory");
        terminated = 1;
        return 0;
    }
    return 1;
}

/* ignores signals or destroy all source calls if they are running */
void signal_handler(int s) {

    if(source_calls)
        source_calls = 0;
    printf("\n");
    signal(s, signal_handler);
}

/*  copy pasta, source: 
    http://cboard.cprogramming.com/c-programming/31839-trim-string-function-code.html 
    trims a string
*/
char *trim ( char *s ) {
  int i = 0;
  int j = strlen ( s ) - 1;
  int k = 0;
 
  while ( isspace ( s[i] ) && s[i] != '\0' )
    i++;
 
  while ( isspace ( s[j] ) && j >= 0 )
    j--;
 
  while ( i <= j )
    s[k++] = s[i++];
 
  s[k] = '\0';
 
  return s;
}

void cleanup_list(char ***list) {
    for(int i = 0; (*list)[i]; i++) {
        free((*list)[i]);
    }
    free(*list);
} 

/*  split a string by every splitchar it encounters,
    put the pieces into *arr, returns the size of arr */
int splitstr(char *str, char ***arr, char splitchar) {
    int max = 5, size = 0, i = 0, j = 0, ignore = 0;
    int str_len = strlen(str);

    *arr = malloc( (sizeof(char *)) * max );
    if(!alloc_check(*arr)) return -1;

    /*  j defines the last place a cut was performed, 
        i the iterator */
    while(i < str_len) {
        do {
            if(str[i] == '"' || str[i] == (char)39 ) // 39 = '
                ignore = ~ignore;
            i++;
        } while((ignore && str[i]) || (str[i] && str[i] != splitchar));

        (*arr)[size] = (char *)malloc( (i-j+1) * sizeof(char));
        strncpy((*arr)[size], (char *)(str + j), i-j);
        (*arr)[size++][i-j] = '\0';
        j = ++i; // skipping the space

        if(size >= max) {
            *arr = realloc(*arr, (max*=2) * sizeof(char *) );
            if(!alloc_check(*arr)) return -1;
        }
    }

    //resize to the filled length + 1 for the NULL (to mak the end of the arr)
    *arr = realloc(*arr, (size + 1) * sizeof(char *));
    if(!alloc_check(*arr)) return -1;

    (*arr)[size] = NULL;
    
    return size;
}

/*  executes an external program as a child (fork), with piping (if necessary) */
int exec_program(char *cmdline, pipe_list_t *pipes) {
    char **args;
    int arg_len, pid;

    arg_len = splitstr(cmdline, &args, ' ');
    if(arg_len > 0) {
        if( (pid = fork()) < 0) {
            perror("Couldn't fork");
            return -1;
        }

        if(!pid) {// child
            // if there is a pipe character after this command, pipe stdout
            if(pipes->curr < pipes->length) {
                if(dup2(curr_pipe(pipes)[1], STDOUT_FILENO) < 0) {
                    perror("Couldn't link output stream");
                }
            }

            // if there was a pipe before this command, make that pipe the stdin
            if(pipes->curr > 0) {
                close(prev_pipe(pipes)[1]);
                if(dup2(prev_pipe(pipes)[0], STDIN_FILENO) < 0) {
                    perror("Couldn't link input stream");
                }
            }

            if(execvp(args[0], args) < 0) {

                if(errno == ENOENT) {
                    printf("%s: command not found\n", args[0]);
                } else {
                    perror(args[0]);
                }
            }

            exit(EXIT_SUCCESS);
        } else {// parent

            if(pipes->curr < pipes->length) {
                close(curr_pipe(pipes)[1]);
            }
            pipes->curr++;
        }
    }

    cleanup_list(&args);
    return pid;
}

/*  Executes a command (with arguments) */
int exec_command(char *cmdline, pipe_list_t *pipes) {
    int cmdline_length = strlen(cmdline), pid = 0;
    
    // check for buildin functions
    for(int i = 0; own_commands[i].func; i++) {
        int cmd_length = strlen(own_commands[i].name);
        if(!strncmp(cmdline, own_commands[i].name, cmd_length) 
        && (cmdline_length <= cmd_length || isspace(cmdline[cmd_length])) ) {

            own_commands[i].func(cmdline);

            return 0;
        }
    }

    /* no buildin, executes program */
    pid = exec_program(cmdline, pipes);
    return pid;
}


/*  Parses an command input into parts (divided by pipe characters) */
void parse_input(char *input) {
    int pipe_count, pid;
    pipe_list_t *pipes;
    char **parts;

    // -1 since you need a pipe for every pipe char, not for every command
    pipe_count = splitstr(input, &parts, '|') - 1;
    pipes = create_pipes(pipe_count);

    for(int i = 0; parts[i]; i++) {
        pid = exec_command(trim(parts[i]), pipes);
        waitpid(pid, NULL, 0);
    }

    cleanup_pipes(pipes);
    cleanup_list(&parts);
}

/*  Gets the input from the terminal screen */
char *get_line_input(FILE *stream) {
    int maxsize = 32, size_left = maxsize;
    char *input = malloc((sizeof(char)) * maxsize);
    char c;

    if(!alloc_check(input)) 
        return input;

    while(!terminated && (c = fgetc(stream)) 
        && c != EOF && c != '\n' && c != '#') {
        input[maxsize - size_left] = c;

        if(--size_left == 0) {
            size_left = maxsize;

            input = realloc(input, maxsize *= 2);

            if(!alloc_check(input)) 
                return input;
        }
    }

    if(c != '\n' && stream == stdin) {
        putchar('\n');
    }

    // no input
    if(!(maxsize - size_left)) {
        free(input);
        return NULL;
    }

    input[maxsize - size_left] = '\0';
    input = realloc(input, maxsize - size_left + 1);
    if(!alloc_check(input))
        return input;

    return input;
}

/*  Runs the shell (prints cwd, asks input, executes the input) */
void init_shell() {
    char *input;
    getcwd(cwd, sizeof(cwd));

    printf("\nWelcome to Vinnie's Virtual OS.\n");
    printf("ViVi will be obedient and is willing to serve\n");
    printf("Please give ViVi a command.\n\n");

    do {
        printf("%s$> ", cwd);
        input = get_line_input(stdin);
        if(!terminated && input) {
            parse_input(input);
        }
        free(input);
    }while(!terminated);
}

/*  Set signal handlers and initializes the shell */
int main() {
    signal (SIGINT, signal_handler);
    signal (SIGQUIT, signal_handler);
    signal (SIGTERM, signal_handler);
    init_shell();
}

// --- buildin functions
/*  Exit the terminal, or the source file */
int do_exit(char *cmd) {
    if(!source_calls) {
        terminated = 1;
    } else {
        source_calls--;
    }
    return 1;
}

/*  Change working directory */
int do_cd(char *cmd) {
    char **args;
    int arg_len = splitstr(cmd, &args, ' ');

    if(arg_len < 1) {
        printf("Not enough arguments supplied\n");
        printf("Usage: %s directory\n",args[0] );
        return 0;
    }

    if(chdir(args[1]) < 0) {
        perror("cd");
    }

    getcwd(cwd, sizeof(cwd));

    cleanup_list(&args);
    
    return 1;
}

/*  Executes a file as terminal commands, line by line */
int do_source(char *cmd) {
    FILE *fp;
    char **args, *line;
    int arg_len = splitstr(cmd, &args, ' ');
    int source_num;

    if(arg_len < 1) {
        printf("Not enough arguments supplied\n");
        printf("Usage: %s file\n",args[0]  );
        return 0;
    }

    if((fp = fopen(args[1],"r")) == NULL) {
        perror(args[0]);
        return 0;
    }

    
    source_num = ++source_calls;

    while(source_calls >= source_num && (line = get_line_input(fp)) ) {
        parse_input(line);
        free(line);
    }

    if(source_calls)
        source_calls--;

    cleanup_list(&args);

    fclose(fp);

    return 1;
}
