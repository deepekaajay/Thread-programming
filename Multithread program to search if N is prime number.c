#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAX_N 100000000
#define MAX_THREADS 25

// global values
int threadNumber;
int largestNumber;
int isPrime;
int nthreads,  // number of threads (not counting main())
    prime[MAX_N + 1],
    n,  // in the end, prime[i] = 1 if i prime, else 0
    nextbase;  // next sieve multiplier to be used

// lock for the shared variable nextbase
pthread_mutex_t nextbaselock = PTHREAD_MUTEX_INITIALIZER;

void crossout(int a) {
    int i, j, check;
    for (i = 2; i < largestNumber; i++)
        prime[i] = 1;

    for (i = a; i < largestNumber;)
        if (prime[i])
            for (j = i; i * j < largestNumber; j++)
                prime[i * j] = 0;

}

int checkPrime(int a) {
    int i;
    for (i = 2; i <= a; ++i) {
        if (a % i == 0) {
            isPrime = 1;
            return isPrime;
            break;
        } else
            isPrime = 2;
        crossout(a);
        return isPrime;
    }
}

void*  workerThread(void* t) {

    int lim, base;

    long i, j;
    long tid;

    tid = (long)t;
    printf("Thread %ld starting...\n", tid);

    while (1)  {


        pthread_mutex_lock(&nextbaselock);
        base = nextbase;
        nextbase++;
        // unlock the lock
        pthread_mutex_unlock(&nextbaselock);
        if (base <= lim)  {

            if (prime[base])  {
                checkPrime(base);
                // log work done by this thread
            }
        }
        if (checkPrime(base) == 2)
            printf("Thread %ld done. Prime = %d\n", tid, base);
        pthread_exit((void*) t);
    }

    return NULL;
}

//main function with two parameters :argc and argv
int main(int argc, char** argv) {

    threadNumber = argv[3];
    largestNumber = argv[1];


    int i;

    pthread_t thread[threadNumber];
    int rc;
    long t;
    void* status;

    for (t = 0; t < threadNumber; t++) {
        printf("Main: creating thread %ld\n", t);
        rc = pthread_create(&thread[t], NULL, workerThread, (void*)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t = 0; t < threadNumber; t++) {
        rc = pthread_join(thread[t], (void*)&t);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        printf("Main: completed join with thread %ld \n", t);
    }
}
