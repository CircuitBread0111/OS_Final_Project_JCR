///////////////////////////|
//|File: client.c
//|Author: Jerrin C. Redmon
//|Language: C
//|Version: 1.0
//|Date: November 28, 2023
///////////////////////////|

/* Description:
 * Clients of the server
 * 200 clients and puts their ID
 */

//----------------------------------------------------------------

// Includes //
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
//
#include <netinet/in.h>



// Main //
int main() {    

    int uid = 0;
    // PUT request
    const char *req_fmt =
    	"PUT / HTTP/1.1\r\n"
    	"Content-length: 8\r\n\r\n"
    	"%8x";
    
    // creates 200 clients
    for (int i = 0; i < 200; i++) {
    	int client_fd = socket(PF_INET, SOCK_STREAM, 0);
		if (client_fd == -1) {
		    fprintf(stderr, "socket: %s\n", strerror(errno));
		    return 1;
		}
		
		// socket adder
		struct sockaddr_in sa = {
		    .sin_family = AF_INET,
		    .sin_port = htons(8080),
		    .sin_addr.s_addr = htonl(0x7f000001) // localhost
		};
    
    	// connects client to server
    	if (connect(client_fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		    fprintf(stderr, "connect: %s\n", strerror(errno));
		    return 1;
		}
    	uid = rand();
    	dprintf(client_fd, req_fmt, uid);
    	char buf[8192];
    	size_t bytes_recvd = read(client_fd, buf, sizeof(buf));
    	buf[bytes_recvd] = 0;
    	puts(buf);
		close(client_fd);
    }
    
}
