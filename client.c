// Hakan Duran 150200091

/*
 * Usage:
 * ./client <host> <port> <file>
 *
 * file parameter will consist the list of 
 * requested files from server
 * 
 * Example usage:
 * ./client localhost 2000 files.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "blg312e.h"

#define MAX_FILENAME_LENGTH 1000

typedef struct { // Struct for clientSend function's parameters
    int arg1;
    char* arg2;
} ThreadParams;

void clientSend(void *args)
{
  // Parameters are assigned to values here
  ThreadParams *params = (ThreadParams *)args;
  int fd = params->arg1;
  char* filename = params->arg2;

  char buf1[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf1, "GET %s HTTP/1.1\n", filename);
  sprintf(buf1, "%s host: %s\n\r\n", buf1, hostname);
  Rio_writen(fd, buf1, strlen(buf1));

  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

  // Allocated params value is freed here
  free(params);

  return NULL;
}

int main(int argc, char *argv[])
{
  char *host, *filename;
  int port;
  int clientfd;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];

  FILE *file = fopen(filename, "r");
    if (file == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", filename);
      exit(1);
    }

  char line[MAX_FILENAME_LENGTH];
  
  
  // In here, each line of file is assigned to "line" variable
  // Connection established with Open_clientfd
  // Parameters are determined and thread created 

  while (fgets(line, sizeof(line), file) != NULL) {

    line[strcspn(line, "\n")] = '\0';

    clientfd = Open_clientfd(host, port);

    ThreadParams *params = malloc(sizeof(ThreadParams));
    params->arg1 = clientfd;
    params->arg2 = strdup(line);
    
    pthread_t tid;
    pthread_create(&tid, NULL, clientSend, (void *)params);

  }

  while(1){
    ;
  }

}