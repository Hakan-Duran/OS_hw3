// Hakan Duran 150200091

/*
 * In my code, i didn't add the code for extra part,
 * which is scheduling policies (FIFO etc).
 */

#include <pthread.h>
#include <semaphore.h>
#include "blg312e.h"
#include "request.h"

sem_t empty; // Semaphore to track empty slots in the buffer
sem_t full;  // Semaphore to track filled slots in the buffer
sem_t mutex; // Mutex for accessing shared buffer
int *buffer; // Shared buffer to hold connection descriptors
int num_buffers = 0; // Maximum number of buffers
int in = 0; // Index for adding items to the buffer
int out = 0; // Index for removing items from the buffer

void *worker(void *arg) {
    int connfd;
    while (1) {
        sem_wait(&full); // Wait if buffer is empty
        sem_wait(&mutex);
        connfd = buffer[out]; // Get connection descriptor from buffer
        out = (out + 1) % num_buffers; // Update index
        sem_post(&mutex);
        sem_post(&empty); // Signal that there's an empty slot in the buffer

        // Handle the request
        requestHandle(connfd);
        Close(connfd);
    }
    return NULL;
}

void getargs(int *port, int *num_threads, int *num_buffers, int argc, char *argv[])
{
    if (argc != 4) {
		fprintf(stderr, "Usage: %s <port> <num_threads> <num_buffers>\n", argv[0]);
		exit(1);
    }
    *port = atoi(argv[1]);
    *num_threads = atoi(argv[2]);
    *num_buffers = atoi(argv[3]);

    if (*num_buffers <= 0) {
      fprintf(stderr, "Number of buffers should be a positive integer.");
      exit(1);
    }

    if (*num_threads <= 0) {
      fprintf(stderr, "Number of threads should be a positive integer.");
      exit(1);
    }
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  int num_threads;

  getargs(&port, &num_threads, &num_buffers, argc, argv);

  buffer = malloc(num_buffers * sizeof(int));
  if (buffer == NULL) {
    fprintf(stderr, "Failed to allocate memory for buffer\n");
    exit(1);
  }

  // Semaphores initialized here
  sem_init(&empty, 0, num_buffers); // It will make thread sleep when buffer is full
  sem_init(&full, 0, 0); // It will make thread sleep when buffer is empty
  sem_init(&mutex, 0, 1); // Locks for producer-consumer relationship

  // Threads created here
  pthread_t tid;
  for (int i = 0; i < num_threads; i++) {
      pthread_create(&tid, NULL, worker, NULL);
  }

  listenfd = Open_listenfd(port);
  while (1) {
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

      sem_wait(&empty); // Wait if buffer is full
      sem_wait(&mutex);
      buffer[in] = connfd; // Put connection descriptor in buffer
      in = (in + 1) % num_buffers; // Update index
      sem_post(&mutex);
      sem_post(&full); // Signal that there's a filled slot in the buffer
  }
}