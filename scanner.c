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


/* The function executeCommand will take (a part of) the command line
   containing a single command (the line has already been cut at the pipe
   symbols), parse that string to find the command and its parameters, and
   then execute that command. */
void executeCommand (unsigned char *commandStr) {
    unsigned char *args[MAX_ARGS] = {NULL};

    /* This is not really elegant, but
       given the fact that we have a limited
       input-line length, it cannot contain
       more than half as many arguments as
       characters */
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
    /* With the use of execvp, it is not necessary to provide the
       absolute path to the executable. The system will automatically
       find the first executable with the requested name in the
       search path environment variable PATH */

    /* Here you should be about ready to call execvp. Do not forget to
       handle the situation where execvp does return */

    exit(-2);
}

void parseCommand (unsigned char *commandStr) {
    unsigned char *pipeChar;

    /* We enter this routine as the parent shell */
    /* If there is no '|' on the command-line, we fork and execute the first
       program, taking care to pass it the appropriate parameters.
       If the command-line has the following structure:
       progr1 par1 par2 | progr2 par3 par4
       or
       progr1 par1 par2 | progr2 par3 par4 | progr3 par5 par6
       the shell should call pipe(), 
       then call fork(), letting the child know that it should connect
       its output to the pipe, and what program it should execute with
       what parameters.
       The code for the child should then do this.
       R:   the parent should then close the writing side of the pipe
       parse the remainder of the command string. 
       If there is another '|', it should once again call pipe()
       and fork again, letting the second child know
       a) to connect its input to the first pipe
       b) if applicable, to connect its output to the second pipe
       c) what program it should execute and with which parameters
       The code for the child should effectuate this
       If this was the final child in the chain, the parent returns, 
       otherwise it jumps back to R:
       N.B. The parent shell should never connect input or output to a pipe,
       such connection should only be made after a fork.
       N.B.2 The above recipe will result in a process tree where all
       programs on the command line are executed by direct descendants of
       the parent shell. For one, this makes it easier to check if all
       such programs have completed before waking up the shell again.
       N.B.3 It may be useful to create a record of the child processes created
       and their IDs
       N.B.4 Remember to undo the effect of sigaction in the child processes -
       it must be possible to terminate such processes by pressing ^C
     */
    /* Now look for the first '|' in the string. We'll be rather obtuse
       about this and disregard " and * and ' and all other usual control
       characters */
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

int scanLine (FILE * fd, int *commandNo) {
    unsigned char commandStr[MAX_LINE];        /* This is not a very elegant
                                           solution. But input lines
                                           should not be infinitely long
                                           anyway */
    int i;
    int rv = 0;

    /* Is stderr the best choice? I'm not sure */
    fprintf(stderr, "myShell[%d]>", *commandNo);
    if (fgets (commandStr, MAX_LINE, fd) == NULL) {

          /* Handle EOF condition and return appropriate value */
    }
    (*commandNo)++;

    /* Check for a builtin */
    for (i = 0; eigen[i].fun; i++) {
          int l = strlen(eigen[i].name);
          if (l == 0)
              break;
          if ((0 == strncmp (commandStr, eigen[i].name, l)) &&
              (isspace (commandStr[l]))) {

                /* I think, we must have a builtin now, so call the
                   appropriate function and return.
                   This procedure will not work for the !n type
                   "builtins"
                   It will also not work if the command is preceded by whitespace */

                return eigen[i].fun (commandStr);
         }
    }

    /* So now we get to the hard part. The commandline specifies one or more
       programs that must be executed. We need to find out if there is a pipe
       symbol, which means that the plumbing must be created correctly */

    parseCommand(commandStr);

    /* The child should not return from parseCommand, I think.
       The code below belongs to the parent - it must return after
       it has finished waiting. Fill rv with some appropriate
       return value. Wait may provide such a value. */

    return rv;
}



/* This is to be the main routine. Start with one or more calls to sigaction to
   ensure that your shell is not vulnerable to a ^C and such.
   Provide alternate handlers for at least SIGINT, SIGQUIT and SIGTERM */


int main(int argc, char *argv[], char *envp[]) {
    char c;
    char *temp = (char *)malloc(sizeof(char) * 100);

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

    /*printf("> ");
	while(c != EOF) {
		c = getchar();
		switch(c) {
            case'\n': 
                if(temp[0] == '\0') {
                    printf("> ");
                } else {
                    
                    
	}
	printf("\n");*/

    while(1) {
        scanLine(, &argc);
    }
	return 0;
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




