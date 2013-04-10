/*
*
*
*
*
*
*
*
*
*
*
*
*/

#include "scanner.h"
#include "piping.h"

#define MAX_ARGS (1024)
#define MAX_LINE (2 * MAX_ARGS)


typedef int builtinFun (char *command);


static int terminate = 0;
static int calls = 0;


struct builtin_Func {
    builtinFun *fun;
    char name[32];
};


int do_exit(char *command);
int do_cd(char *command);
int do_source(char *command);


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


int check_allocation(void *pntr) {
    if(!pntr) {
        perror("Couldn't allocate memory");
        terminate = 1;
        return 0;
    }
    return 1;
}


/*
* trimwhitespace function from http://tinyurl.com/trimwhitespacestandard
*/
char *trimwhitespace(char *str) {
    char *end;

    while(isspace(*str)) str++;

    if(*str == 0) {
        return str;
    }

    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) {
        end--;
    }
    *(end+1) = 0;

    return str;
}


void executeCommand (unsigned char *commandStr) {
    unsigned char *args[MAX_ARGS] = {NULL};


    int i = 1;
    args[0] = strtok(commandStr, " \t\n");
    while ((args[i] = strtok(NULL, " \t\n"))) {
        i++;
    }
    if (strchr(args[0], '/')) {
          fprintf(stderr, "Attempt to call function '%s' not"
                          "in the PATH environment variable\n", args[0]);
          exit(-1);
    }

    exit(-2);
}

void parseCommand (unsigned char *commandStr) {
    unsigned char *pipeChar;


    if ((pipeChar = strchr (commandStr, '|')))
      {
          unsigned char commandStr1[MAX_LINE];
          unsigned char *cpntr = commandStr;
          unsigned char *cpntr1 = commandStr1;

          while (cpntr != pipeChar)
            {
                *(cpntr1++) = *(cpntr++);
            }
          *cpntr1 = 0;
          executeCommand (commandStr1);

          parseCommand (pipeChar + 1);
      }

    executeCommand(commandStr);
}




int scanLine(FILE *fd) {
    int maxsize = 32, size_left = maxsize;
    char *input = malloc((sizeof(char)) * maxsize);
    char c;

    if(!alloc_check(input)) 
        return input;

    while(!terminate && (c = fgetc(fd)) 
        && c != EOF && c != '\n' && c != '#') {
        input[maxsize - size_left] = c;

        if(--size_left == 0) {
            size_left = maxsize;

            input = realloc(input, maxsize *= 2);

            if(!alloc_check(input)) 
                return input;
        }
    }

    if(c != '\n' && fd == stdin) {
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


void run_shell() {
    char *input;
    //getcwd(cwd, sizeof(cwd));

    do {
        printf("myShell>");
        input = scanline(stdin);
        if(!terminate && input) {
            parseCommand(input);
        }
        free(input);
    }while(!terminate);
}


int main(int argc, char *argv[], char *envp[]) {
    struct sigaction new_sa, new_sa_2, new_sa_3;
    struct sigaction old_sa, old_sa_2, old_sa_3;

    sigfillset(&new_sa.sa_mask);
    new_sa.sa_handler = SIG_IGN;
    new_sa.sa_flags = 0;
    sigaction(SIGINT, &new_sa, 0); 

    sigfillset(&new_sa_2.sa_mask);
    new_sa_2.sa_handler = SIG_IGN;
    new_sa_2.sa_flags = 0;
    sigaction(SIGTERM, &new_sa_2, 0); 

    
    sigfillset(&new_sa_3.sa_mask);
    new_sa_3.sa_handler = SIG_IGN;
    new_sa_3.sa_flags = 0;
    sigaction(SIGQUIT, &new_sa_3, 0);    

    run_shell();
}



int do_exit (char *command){
    if(!calls) {
        terminate = 1;
    } else {
        calls--;
    }
    return 1;
}


int do_cd(char *command){
    char **args;
    int arg_len = splitstr(command, &args, ' ');

    if(arg_len < 1) {
        printf("Too few arguments\n");
        printf("Using: %s directory\n", args[0]);
        return 0;
    }

    if(chdir(args[1]) < 0) {
        perror("chdir() failed");
    }

    getcwd(cwd, sizeof(cwd));

    free_array(&args);
    
    return 1;
}


int do_source(char *command){
    FILE *fp;
    char **args, *line;
    int arg_len = splitstr(command, &args, ' ');
    int source_num;

    if(arg_len < 1) {
        printf("Wrong amount of arguments\n");
        printf("Using: %s file\n",args[0]);
        return 0;
    }

    if((fp = fopen(args[1],"r")) == NULL) {
        perror(args[0]);
        return 0;
    }

    source_num = ++source_calls;

    while(source_calls >= source_num && (line = get_line_input(fp)) ) {
        parseCommand(line);
        free(line);
    }

    if(source_calls) {
        source_calls--;
    }

    free_array(&args);

    fclose(fp);

    return 1;
}  




