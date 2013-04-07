/* This skeleton code is provided for the practical work for the "BS"
   course 2012-2013 at the Universiteit van Amsterdam. It is untested code that
   shows how a command line could possibly be parsed by a very simple
   shell.
   (C) Universiteit van Amsterdam, 1997 - 2013
   Author: G.D. van Albada
           G.D.vanAlbada@uva.nl
   Date:   October 1, 1997

   At least one problem is known: a command string containg the (illegal)
   combination '||' will lead to severe problems - so add extra tests.
   */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#define MAX_ARGS        (1024)
#define MAX_LINE        (2 * MAX_ARGS)

typedef void (*sighandler_t)(int);
char c = '\0';
static char* cmd_line_args[100];
static int terminate = 0;

/* define a builtinFun type for later use */
typedef int builtinFun (unsigned char *command);

int do_exit (unsigned char *command);
int do_cd (unsigned char *command);
int do_source (unsigned char *command);

/* define a table associating functions with commands */
typedef struct builtin {
    builtinFun *fun;
    unsigned char name[32];
} builtin;

static builtin eigen[] = {
    {do_exit, "exit"},
    {do_cd, "cd"},
    {do_source, "source"},
    {do_source, "."},
    {NULL, ""}
};



void executeCommand (unsigned char *commandStr) {
    unsigned char *args[MAX_ARGS] = {NULL};


    int i = 1;
    args[0] = strtok (commandStr, " \t\n");
    while ((args[i] = strtok (NULL, " \t\n"))) {
        i++;
    }
    if (strchr (args[0], '/')) {
          fprintf (stderr, "Attempt to call function '%s' not"
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
          unsigned char *cptr = commandStr;
          unsigned char *cptr1 = commandStr1;

          /* There appears to be a '|', so we must make a pipe and 
             do all the required file manipulation here. See man pages
             for dup2 and pipe. */

          /* Here we fork, I think. One process, e.g. the child
             will execute the first command, the other will parse the
             remainder of the commandstring */

          /* Process A: prepare to execute first command */
          /* Now construct command string for the first command */
          while (cptr != pipeChar)
            {
                *(cptr1++) = *(cptr++);
            }
          *cptr1 = 0;
          executeCommand (commandStr1);

          /* Process B: Continue parsing recursively where we left off */
          parseCommand (pipeChar + 1);
      }

    /* No '|' left in string - just execute command */
    executeCommand (commandStr);
}

int scanLine (FILE * fd) {
    unsigned char commandStr[MAX_LINE];       
    int i;
    int rv = 0;

    if (fgets (commandStr, MAX_LINE, fd) == NULL) {

    }

    for (i = 0; eigen[i].fun; i++) {
        int l = strlen(eigen[i].name);
        if (l == 0)
              break;
        if ((0 == strncmp (commandStr, eigen[i].name, l)) &&
           (isspace (commandStr[l]))) {
            return eigen[i].fun (commandStr);
        }
    }

    parseCommand(commandStr);

    return rv;
}


void run_shell() {
    char *input;
    //getcwd(cwd, sizeof(cwd));

    do {
        printf("myShell>");
        input = scanline(stdin);
        if(!terminate && input) {
            parse_input(input);
        }
        free(input);
    }while(!terminated);
}


int main(int argc, char *argv[], char *envp[]) {

    struct sigaction new_sa;
    struct sigaction old_sa;
    sigfillset(&new_sa.sa_mask);
    new_sa.sa_handler = SIG_IGN;
    new_sa.sa_flags = 0;
    sigaction(SIGINT, &new_sa, 0); 

    struct sigaction new_sa_2;
    struct sigaction old_sa_2;
    sigfillset(&new_sa_2.sa_mask);
    new_sa_2.sa_handler = SIG_IGN;
    new_sa_2.sa_flags = 0;
    sigaction(SIGTERM, &new_sa_2, 0); 

    struct sigaction new_sa_3;
    struct sigaction old_sa_3;
    sigfillset(&new_sa_3.sa_mask);
    new_sa_3.sa_handler = SIG_IGN;
    new_sa_3.sa_flags = 0;
    sigaction(SIGQUIT, &new_sa_3, 0);    

    run_shell();
}



int do_exit (unsigned char *command){
    return 0;
}


int do_cd (unsigned char *command){
    return 0;
}


int do_source (unsigned char *command){
    return 0;
}




