///////////////////////////|
//|File: server.c
//|Author: Jerrin C. Redmon
//|Language: C
//|Version: 1.0
//|Date: November 28, 2023
///////////////////////////|

/* Description:
 * a multithreaded http server
 * that uses mutex and connects
 * 200 clients and puts their ID
 */

//----------------------------------------------------------------

// Includes //
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
//
#include <netinet/in.h>
// 
#include "queue.h"


static Queue *connection_queue;									// Connection Queue
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER; // PThread mutex


// Connection data //
struct connection_data {
    volatile int client_fd; 	// client file descriptor
    volatile int id;			// client id
};


// Add connection //
// params: id
static void add_connection(int id) {

	pthread_mutex_lock(&queue_mutex);		// create thread
	Queue_Enqueue(connection_queue, id); 	// enqueue id
	pthread_mutex_unlock(&queue_mutex); 	// unlock mutex
}


// Connection thread //
static void *connection_thread(void *arg) { // arg MUST BE malloced

    struct connection_data *cdata = arg;
    char req_data[8192]; // requested data 
    ssize_t req_size = read(cdata->client_fd, req_data, sizeof(req_data) - 1);
    int content_length;
	int uid; // user id
	if (
		sscanf(req_data, "PUT / HTTP/1.1\r\nContent-length: %d\r\n\r\n%8x",
		&content_length, &uid
	)) {
		add_connection(uid);
	}
	close(cdata->client_fd);
    free(cdata); // free connection data after request
    return NULL;
}


// Main //
int main(void) {
	
	// creates socket
    int server_fd = socket(PF_INET, SOCK_STREAM, 0); // server file descriptor
    if (server_fd == -1) {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return 1;
    }
    
	// socket address
    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port = htons(8080), //localhost 
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    
	// bind socket
    if (bind(server_fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        return 1;
    }
    
	// socket listener
    if (listen(server_fd, 128) == -1) {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        return 1;
    }
	
	connection_queue = Queue_Create(1024);
	long num_responses = 0;
    for (int i = 0; i <= 200; i++) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd != -1) {
            pthread_t pt;
            // allocates cdata
            struct connection_data *cdata = malloc(sizeof(*cdata));
            cdata->client_fd = client_fd;
            cdata->id = num_responses++;
            pthread_create(&pt, NULL, &connection_thread, cdata); 
        }
    }
    
    // Dequeues clients, prints id
    printf("size: %zd\n", Queue_Size(connection_queue));
    while (!Queue_IsEmpty(connection_queue)) {
    	int id;
    	Queue_Dequeue(connection_queue, &id);
    	printf("%x\n", id);
    }
    // close server after finished
    close(server_fd);
}
