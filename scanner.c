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




static int terminate = 0;
static int calls = 0;
static char chwd[2048];



static struct builtin_Func eigen[] = {
    {do_exit, "exit"},
    {do_cd, "cd"},
    {do_source, "source"},
    {do_source, "."},
    {NULL, ""}
};


void free_arrayay(char ***arrayay) {
    for(int i = 0; (*arrayay)[i]; i++) {
        free((*arrayay)[i]);
    }
    free(*arrayay);
} 

int split_stringing(char *string, char ***array, char split_character) {
    int max = 5, size = 0, i = 0, j = 0, ignore = 0;
    int stringing_length = strlen(string);

    *array = malloc( (sizeof(char *)) * max );
    if(!check_allocation(*array)) return -1;

    /*  j defines the last place a cut was performed, 
        i the iterator */
    while(i < stringing_length) {
        do {
            if(string[i] == '"' || string[i] == (char)39 ) // 39 = '
                ignore = ~ignore;
            i++;
        } while((ignore && string[i]) || 
                (string[i] && string[i] != split_character));

        (*array)[size] = (char *)malloc( (i-j+1) * sizeof(char));
        strncpy((*array)[size], (char *)(string + j), i-j);
        (*array)[size++][i-j] = '\0';
        j = ++i; // skipping the space

        if(size >= max) {
            *array = realloc(*array, (max*=2) * sizeof(char *) );
            if(!check_allocation(*array)) return -1;
        }
    }

    //resize to the filled length + 1 for the NULL (to mak the end of the array)
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
* trimwhitespace function from http://tinyurl.com/trimwhitespacestandard
*/
char *trimwhitespace(char *string) {
    char *end;

    while(isspace(*string)) string++;

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


void executeCommand (unsigned char *commandstring) {
    unsigned char *args[MAX_ARGS] = {NULL};


    int i = 1;
    args[0] = strtok(commandstring, " \t\n");
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

void parseCommand (unsigned char *commandstring) {
    unsigned char *pipeChar;


    if ((pipeChar = strchr (commandstring, '|')))
      {
          unsigned char commandstring1[MAX_LINE];
          unsigned char *cpntr = commandstring;
          unsigned char *cpntr1 = commandstring1;

          while (cpntr != pipeChar)
            {
                *(cpntr1++) = *(cpntr++);
            }
          *cpntr1 = 0;
          executeCommand (commandstring1);

          parseCommand (pipeChar + 1);
      }

    executeCommand(commandstring);
}




int scanLine(FILE *fd) {
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

    // no input
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
    getcwd(chwd, sizeof(chwd));

    do {
        printf("myShell>");
        input = scanLine(stdin);
        if(!terminate && input) {
            parseCommand(input);
        }
        free(input);
    }while(!terminate);
}


int main(int argc, char *argv[], char *envp[]) {
    /*struct sigaction new_sa, new_sa_2, new_sa_3;
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
    sigaction(SIGQUIT, &new_sa_3, 0);*/
    
    signal(SIGINT, signal_handler);    
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);

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
    int arg_len = split_stringing(command, &args, ' ');

    if(arg_len < 1) {
        printf("Too few arguments\n");
        printf("Using: %s directory\n", args[0]);
        return 0;
    }

    if(chdir(args[1]) < 0) {
        perror("chdir() failed");
    }

    getcwd(chwd, sizeof(chwd));

    free_arrayay(&args);
    
    return 1;
}


int do_source(char *command){
    FILE *fp;
    char **args, *line;
    int arg_len = split_stringing(command, &args, ' ');
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

    source_num = ++calls;

    while(calls >= source_num && (line = scanLine(fp)) ) {
        parseCommand(line);
        free(line);
    }

    if(calls) {
        calls--;
    }

    free_arrayay(&args);

    fclose(fp);

    return 1;
}  




