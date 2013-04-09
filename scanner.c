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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>



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


int check_allocation(void *ptr) {
    if(!ptr) {
        perror("Couldn't allocate memory");
        terminate = 1;
        return 0;
    }
    return 1;
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
    executeCommand(commandStr);
}

int scanLine(FILE *fd) {
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
    }while(!terminate);
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



int do_exit (char *command){
    if(!calls) {
        terminate = 1;
    } else {
        calls--;
    }
    return 1;
}


int do_cd(char *command){
    return 0;
}


int do_source(char *command){
    return 0;
}




