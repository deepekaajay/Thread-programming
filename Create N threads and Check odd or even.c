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

int odd_counter, even_counter;
pthread_mutex_t mutex;
pthread_cond_t cond;


void* worker(void* arg) {
	int value = rand() % 1000 + 1;	
	printf("Sorted %d\n", value);
	pthread_mutex_lock(&mutex);
	if(value % 2 == 0)
		even_counter++;
	else odd_counter++;
	pthread_mutex_unlock(&mutex);
	
	pthread_cond_signal(&cond);
}

int main(int argc, char** argv) {
	
	int rc,i,N;
	pthread_t *threads;
	pthread_attr_t attr;
	char s_err[50];
	
	memset(s_err, 0, 50);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&cond, NULL);
	
	if(argc != 2) {
		fprintf(stderr, "Wrong arguments, usage: <n_threads>\n");
		return 1;
	}
	N = atoi(argv[1]);
	srand(time(NULL));		// to get different sequences of randoms
	
	for(i=0, threads = NULL; i<N; i++) {
		threads = realloc(threads, (i+1)*sizeof(pthread_t));
		if(threads == NULL) {
			fprintf(stderr, "Realloc error\n");
			return 1;
		}
		rc = pthread_create(&threads[i], &attr, worker, NULL);
		if(rc > 0) {
			perror(s_err);
			fprintf(stderr, "%s\n", s_err);
			return 1;
		}
	}
	
	pthread_mutex_lock(&mutex);
	while(odd_counter + even_counter < N) {
		pthread_cond_wait(&cond, &mutex);
	}
	printf("Odd: %d\t\tEven: %d\n", odd_counter, even_counter);
	pthread_mutex_unlock(&mutex);
	
	pthread_attr_destroy(&attr);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	free(threads);
	
	return 0;

}
