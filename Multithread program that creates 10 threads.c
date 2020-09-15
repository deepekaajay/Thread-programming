#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

double x, y;

void* evaluate_x(void* arg) {}

void* evaluate_y(void* arg) {}

int main(int argc, char **argv) {
	int rc, N, i;
	pthread_t *threads;
	int *th_arg;
	char s_err[50];
	
	memset(s_err, 0, 50);
	
	if(argc != 4) {
		// TODO
	}
	x = atof(argv[1]);
	y = atof(argv[2]);
	N = atoi(argv[3]);
	
	threads = malloc(2*N*sizeof(pthread_t));
	if(threads == NULL) {
		//TODO
	}
	for(i=0; i<N; i++) {
		
		th_arg = malloc(sizeof(int));			// pass i to thread
		if(th_arg == NULL) {
			fprintf(stderr, "Malloc error\n");
			return 1;		
		}
		*th_arg = i;
		
		rc = pthread_create(&thread, NULL, writer, (void*)th_arg);
		if(rc > 0) {
			perror(s_err);
			fprintf(stderr,"%s\n", s_err);
			return 1;
		}
	}
	
		
	}
	
}
