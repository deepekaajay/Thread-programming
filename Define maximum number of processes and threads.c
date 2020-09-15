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

pthread_mutex_t mutex;		

void* do_nothing (void* arg) {

	// mutex needed to keep threads alive until the end of the creation loop
	
	pthread_mutex_lock(&mutex);
	pthread_exit(NULL);
}

int main() {
	
	int i, r;
	pthread_attr_t attr;
	pthread_t *thread;
	char s_err[50];
	clock_t begin, end;
	pid_t pid;
	int number_of_processes;
	
	memset(s_err, 0, 50);
	// detach state to avoid joining in order to free memory
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&mutex, NULL);
	thread = NULL;
	
	begin = clock();
	
	pthread_mutex_lock(&mutex);
	for(i=0; 1; i++) {
		thread = realloc(thread, (i+1)*sizeof(pthread_t));	// resize array each time a thread is created
		if(thread == NULL) {
			fprintf(stderr, "Error in realloc\n");
			return -1;
		}
		r = pthread_create(&thread[i], &attr, do_nothing, NULL);
		printf("Created thread %d\n", i+1);
		if(r != 0) {
			if(r == EAGAIN) {
				printf("Limit reached\n");
				break;
			} else {
				perror(s_err);
				fprintf(stderr, "%s\n", s_err);
				return -1;
			}
		}
	
	}
	pthread_mutex_unlock(&mutex);
	
	end = clock();
	printf("Thread limit: %d\n", i+1);
	printf("Time elapsed: %f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);
	
	free(thread);
	pthread_attr_destroy(&attr);
	
	number_of_processes = 1;
	do {
		pid = fork();
		//sleep(1);
		if(pid < 0) {
			
			printf("Limit of processes: %d\n", number_of_processes);
				
			exit(0);
		} else if (pid == 0) {	// child
			number_of_processes++;
			printf("Created process %d\n", number_of_processes);
		}
	} while (pid == 0);
	waitpid(pid, NULL, 0);
	
	pthread_exit(NULL);
}
