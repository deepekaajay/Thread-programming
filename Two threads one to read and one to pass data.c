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
#include <semaphore.h>


char* input_file;
char buffer;			// shared buffer (producer-consumer pattern)  
sem_t sem_ask_next_files, sem_wait_next_files, sem_buffer_full, sem_buffer_empty;

void* reader(void* arg) {
	int fd,rc;
	char s_err[50];
	
	memset(s_err, 0, 50);
	
	while(1) {
		sem_wait(&sem_wait_next_files);
		if(strcmp(input_file, "END") == 0) {
			// this sem_post is used to wake up the writer when END command occurs, giving it the possibility to break
			 // its infinite loop and terminate.
			sem_post(&sem_buffer_empty);
			break;
		} else {
			printf("Opening file %s....\n", input_file);
			fd = open(input_file, O_RDONLY);
			if(fd < 0) {
				perror(s_err);
				fprintf(stderr, "%s\n", s_err);
				exit(1);
			}
			do {
				sem_wait(&sem_buffer_full);					// waiting the consumer (writer thread)
				rc = read(fd, &buffer, sizeof(char));		// reading from file
				//printf("%c", buffer);
				if(rc < 0) {
					perror(s_err);
					fprintf(stderr, "%s\n", s_err);
					exit(1);
				} 
				sem_post(&sem_buffer_empty);
			} while (rc > 0);
			/* when the reader is signalled for the last written character, and it reaches the EOF, it lets the main thread asking
			* for the new input file. */
			close(fd);
			sem_post(&sem_ask_next_files);
		}
	}
	
	printf("Reader end\n");
	pthread_exit(NULL);
	
}


void* writer(void* arg) {
	int fd,rc;
	char s_err[50];
	char* output_file;
	
	output_file = (char*)arg;
	printf("Opening %s...\n", output_file);
	memset(s_err, 0, 50);
	fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(fd < 0) {
		perror(s_err);
		fprintf(stderr, "%s\n", s_err);
		exit(1);
	}
	while(1) {
		sem_wait(&sem_buffer_empty);		// wait for something to write
		if(strcmp(input_file,"END") == 0) {
			close(fd);
			break;
		} else {
			write(fd, &buffer, sizeof(char));	
			printf("%c", buffer);
			sem_post(&sem_buffer_full);
		}
	}
	printf("Writer end...\n");
	pthread_exit(NULL);
	
}


int main(int argc, char** argv) {

	int rc;
	pthread_t reader_id, writer_id;
	char s_err[50];
	char* output_file;
	
	if(argc != 3) {
		fprintf(stderr, "Wrong number of arguments, usage: <input_file>, <output_file>\n");
		return 1;
	}
	input_file = argv[1];
	output_file = argv[2];
	memset(s_err, 0, 50);
	
	/* two semaphore to implement producer-consumer between the main thread and the reader */
	sem_init(&sem_ask_next_files, 0, 0);			// initially we already know files, so we have to wait to ask them
	sem_init(&sem_wait_next_files, 0, 1);			// initially we can start reading from files
	
	/* two semaphore to implement producer-consumer between the writer thread thread and the reader */
	sem_init(&sem_buffer_empty, 0, 0);			
	sem_init(&sem_buffer_full, 0, 1);
	
	rc = pthread_create(&reader_id, NULL, reader, NULL);
	if(rc > 0) {
		perror(s_err);
		fprintf(stderr,"%s\n", s_err);
		return 1;
	}
	rc = pthread_create(&writer_id, NULL, writer, (void*)output_file);
	if(rc > 0) {
		perror(s_err);
		fprintf(stderr,"%s\n", s_err);
		return 1;
	}
	
	do {
		sem_wait(&sem_ask_next_files);
		printf("Insert separating by space <input file>: ");
		fflush(stdin);
		if(scanf("%s", input_file) <= 0) {
			fprintf(stderr, "Scanf error\n");
			return 1;
		}
		sem_post(&sem_wait_next_files);		
	} while (strcmp(input_file, "END") != 0);
	
	pthread_join(reader_id,NULL);
	pthread_join(writer_id,NULL);
	
	printf("Main thread is over\n");
	sem_destroy(&sem_ask_next_files);
	sem_destroy(&sem_wait_next_files);
	sem_destroy(&sem_buffer_empty);
	sem_destroy(&sem_buffer_full);
	
	return 0;
	
}
