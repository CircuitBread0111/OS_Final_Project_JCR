///////////////////////////|
//|File: server.c
//|Author: Jerrin C. Redmon
//|Language: C
//|Version: 1.0
//|Date: November 28, 2023
///////////////////////////|

/* Description:
 * a multithreaded http server
 * DOES NOT USE MUTEX - FOR TESTING ONLY
 * 200 clients and puts their ID
 */

//----------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include "queue.h"

// no mutex got lucky

static Queue *connection_queue;

struct connection_data {
    volatile int client_fd;
    volatile int id;
};

static void add_connection(int id) {
	// things in here
	Queue_Enqueue(connection_queue, id); //true
}

// arg MUST BE malloced
static void *connection_thread(void *arg) {
    struct connection_data *cdata = arg;

    char req_data[8192];
    ssize_t req_size = read(cdata->client_fd, req_data, sizeof(req_data) - 1);
    int content_length;
	int uid;
	if (
		sscanf(req_data, "PUT / HTTP/1.1\r\nContent-length: %d\r\n\r\n%8x",
		&content_length, &uid
	)) {
		add_connection(uid);
	}
	
	close(cdata->client_fd);
    free(cdata);
    return NULL;
}

int main(void) {
    int server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return 1;
    }

    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if (bind(server_fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        return 1;
    }

    if (listen(server_fd, 128) == -1) {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        return 1;
    }
	
	connection_queue = Queue_Create(1024);
	long num_responses = 0;
    for (int i = 0; i < 200; i++) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd != -1) {
            pthread_t pt;
            struct connection_data *cdata = malloc(sizeof(*cdata));
            cdata->client_fd = client_fd;
            cdata->id = num_responses++;
            pthread_create(&pt, NULL, &connection_thread, cdata);
        }
    }
    
    printf("size: %zd\n", Queue_Size(connection_queue));
    while (!Queue_IsEmpty(connection_queue)) {
    	int id;
    	Queue_Dequeue(connection_queue, &id);
    	printf("%x\n", id);
    }
    
    close(server_fd);
}
