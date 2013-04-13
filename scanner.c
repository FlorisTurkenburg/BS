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

#include "scanner.h"
#include "piping.h"

#define MAX_arguments (1024)
#define MAX_LINE (2 * MAX_arguments)


static int terminate = 0;
static int calls = 0;
static char cwd[2048];


static struct builtin_Func eigen[] = {
    {do_exit, "exit"},
    {do_cd, "cd"},
    {do_source, "source"},
    {do_source, "."},
    {NULL, ""}
};


void free_array(char ***array) {
    for(int i = 0; (*array)[i]; i++) {
        free((*array)[i]);
    }
    free(*array);
} 

int split_string(char *string, char ***array, char split_character) {
    int max = 5;
    int size = 0;
    int i = 0;
    int j = 0;
    int ignore = 0;
    int string_length = strlen(string);

    *array = malloc((sizeof(char *)) * max);
    if(!check_allocation(*array)) {
        return -1;
    }
    while(i < string_length) {
        do {
            if(string[i] == '"' || string[i] == (char)39 )
                ignore = ~ignore;
            i++;
        } while((ignore && string[i]) || 
                (string[i] && string[i] != split_character));

        (*array)[size] = (char *)malloc( (i-j+1) * sizeof(char));
        strncpy((*array)[size], (char *)(string + j), i-j);
        (*array)[size++][i-j] = '\0';
        j = ++i;

        if(size >= max) {
            *array = realloc(*array, (max*=2) * sizeof(char *) );
            if(!check_allocation(*array)) return -1;
        }
    }

    *array = realloc(*array, (size + 1) * sizeof(char *));
    if(!check_allocation(*array)) return -1;

    (*array)[size] = NULL;
    
    return size;
}


int check_allocation(void *pntr) {
    if(!pntr) {
        perror("Couldn't allocate memory");
        terminate = 1;
        return 0;
    }
    return 1;
}

void signal_handler(int s) {

    if(calls)
        calls = 0;
    printf("\n");
    signal(s, signal_handler);
}


/*
* trim_whitespace function from http://tinyurl.com/trim_whitespacestandard
*/
char *trim_whitespace(char *string) {
    char *end;

    while(isspace(*string)){
        string++;
    }

    if(*string == 0) {
        return string;
    }

    end = string + strlen(string) - 1;
    while(end > string && isspace(*end)) {
        end--;
    }
    
    *(end+1) = 0;

    return string;
}


int execute_program(char *command_line, pipes_list *pipes) {
    char **arguments;
    int arg_len, pid;

    arg_len = split_string(command_line, &arguments, ' ');
    if(arg_len > 0) {
        if((pid = fork()) < 0) {
            perror("Fork failed");
            return -1;
        }

        if(!pid) {
            if(pipes->current < pipes->length) {
                if(dup2(current_pipe(pipes)[1], STDOUT_FILENO) < 0) {
                    perror("Link to output stream failed");
                }
            }

            if(pipes->current > 0) {
                close(previous_pipe(pipes)[1]);
                if(dup2(previous_pipe(pipes)[0], STDIN_FILENO) < 0) {
                    perror("Link to input stream failed");
                }
            }

            if(execvp(arguments[0], arguments) < 0) {
                if(errno == ENOENT) {
                    printf("%s: command not found\n", arguments[0]);
                } else {
                    perror(arguments[0]);
                }
            }

            exit(EXIT_SUCCESS);
        } else {
            if(pipes->current < pipes->length) {
                close(current_pipe(pipes)[1]);
            }
            pipes->current++;
        }
    }

    free_array(&arguments);
    return pid;
}


int execute_command(char *command_line, pipes_list *pipes) {
    int command_line_length = strlen(command_line), pid = 0;

    for(int i = 0; eigen[i].fun; i++) {
        int command_length = strlen(eigen[i].name);
        if(!strncmp(command_line, eigen[i].name, command_length) 
        && (command_line_length <= command_length || isspace(command_line[command_length]))) {

            eigen[i].fun(command_line);

            return 0;
        }
    }

    pid = execute_program(command_line, pipes);
    return pid;
}

void parse_command(char *command_string) {
    int pipe_count, pid;
    pipes_list *pipes;
    char **segments;

    pipe_count = split_string(command_string, &segments, '|') - 1;
    pipes = make_pipes(pipe_count);

    for(int i = 0; segments[i]; i++) {
        pid = execute_command(trim_whitespace(segments[i]), pipes);
        waitpid(pid, NULL, 0);
    }

    free_pipes(pipes);
    free_array(&segments);
}


char *scan_line(FILE *fd) {
    int maxsize = 32, size_left = maxsize;
    char *input = malloc((sizeof(char)) * maxsize);
    char c;

    if(!check_allocation(input)) 
        return input;

    while(!terminate && (c = fgetc(fd)) 
        && c != EOF && c != '\n' && c != '#') {
        input[maxsize - size_left] = c;

        if(--size_left == 0) {
            size_left = maxsize;

            input = realloc(input, maxsize *= 2);

            if(!check_allocation(input)) 
                return input;
        }
    }

    if(c != '\n' && fd == stdin) {
        putchar('\n');
    }

    if(!(maxsize - size_left)) {
        free(input);
        return NULL;
    }

    input[maxsize - size_left] = '\0';
    input = realloc(input, maxsize - size_left + 1);
    if(!check_allocation(input))
        return input;

    return input;
}


void run_shell() {
    char *input;
    getcwd(cwd, sizeof(cwd));

    do {
        printf("%s--SHELL>", cwd);
        input = scan_line(stdin);
        if(!terminate && input) {
            parse_command(input);
        }
        free(input);
    }while(!terminate);
}


int main(int argc, char *argv[], char *envp[]) {
    signal(SIGINT, signal_handler);    
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);

    run_shell();
}



int do_exit(char *command){
    if(!calls) {
        terminate = 1;
    } else {
        calls--;
    }
    return 1;
}


int do_cd(char *command){
    char **arguments;
    int arg_len = split_string(command, &arguments, ' ');

    if(arg_len < 1) {
        printf("Too few arguments\n");
        printf("Using: %s directory\n", arguments[0]);
        return 0;
    }

    if(chdir(arguments[1]) < 0) {
        perror("chdir() failed");
    }

    getcwd(cwd, sizeof(cwd));

    free_array(&arguments);
    
    return 1;
}


int do_source(char *command){
    FILE *fp;
    char **arguments, *line;
    int arg_len = split_string(command, &arguments, ' ');
    int number_source;

    if(arg_len < 1) {
        printf("Wrong amount of arguments\n");
        printf("Using: %s file\n",arguments[0]);
        return 0;
    }

    if((fp = fopen(arguments[1],"r")) == NULL) {
        perror(arguments[0]);
        return 0;
    }

    number_source = ++calls;

    while(calls >= number_source && (line = scan_line(fp)) ) {
        parse_command(line);
        free(line);
    }

    if(calls) {
        calls--;
    }

    free_array(&arguments);

    fclose(fp);

    return 1;
}  




