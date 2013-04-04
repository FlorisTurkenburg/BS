#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef void (*sighandler_t)(int);
char c = '\0';

void ignore( int sig )
{            
    fprintf(stderr, "\n"); // Print a new line
                            // This function does nothing except ignore ctrl-c
}

int main(int argc, char *argv[]) {
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

    printf("> ");
	while(c != EOF) {
		c = getchar();
		if(c == '\n')
			printf("> ");
	}
	printf("\n");
	return 0;
}
