#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef void (*sighandler_t)(int);
char c = '\0';

void handle_signal(int signo)
{
	printf("\n> ");
	fflush(stdout);
}

int main(int argc, char *argv[], char *envp[])
{
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);
	printf("> ");
	while(c != EOF) {
		c = getchar();
		if(c == '\n')
			printf("> ");
	}
	printf("\n");
	return 0;
}
