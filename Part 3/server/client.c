///////////////////////////|
//|File: client.c
//|Author: Jerrin C. Redmon
//|Language: C
//|Version: 1.0
//|Date: November 28, 2023
///////////////////////////|

/* Description:
 * Creates a client for the server
 * connects client to a simple game of
 * Rock Paper Scissors Online!
 * With other clients!
 */

//----------------------------------------------------------------

// Includes // 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Defines //
#define _GNU_SOURCE

// Connection states /
struct connection_state {
	int fd; 				// File descripter
	struct sockaddr_in sa;  // Socket address
};

// HTTP request type //
enum req_type { REQ_GET, REQ_PUT };


// Send request //
// param: connection, request type, uri
static void send_request(struct connection_state *conn, int req_type, const char *uri) {

	conn->fd = socket(PF_INET, SOCK_STREAM, 0);
	if (conn->fd == -1) {
	    fprintf(stderr, "socket: %s\n", strerror(errno));
	    exit(1);
	}
	if (connect(conn->fd, (struct sockaddr *) &conn->sa, sizeof(conn->sa)) == -1) {
	    fprintf(stderr, "connect: %s\n", strerror(errno));
	    exit(1);
	}
	const char *REQ_STRINGS[2] = { "GET", "PUT" };
	dprintf(conn->fd, "%s %s HTTP/1.1\r\n\r\n", REQ_STRINGS[req_type], uri);
}


// Recieve response //
// param: connection
// malloc the returned string! free this!
static char *recv_response(struct connection_state *conn) {

	char res_data[8192]; 	// allowed requested data amount of 8192 characters
    ssize_t res_size = 0;	// size of requested data set to 0
	int chars_scanned; 		// characters scanned
	int status; 			// response status
	
    // checks response data
	do {
    	res_size += read(conn->fd, res_data + res_size, 1);
	} while (res_size < 4 || (
		res_data[res_size - 4] != '\r' ||
    	res_data[res_size - 3] != '\n' ||
    	res_data[res_size - 2] != '\r' ||
    	res_data[res_size - 1] != '\n'
    ));
	res_data[res_size] = '\0';
	// scan response
	if (sscanf(res_data, "HTTP/1.1 %d %*s\r\n%n", &status, &chars_scanned) != 1) {
		fprintf(stderr, "Invalid HTTP response\n");
		exit(1);
	}
	char *retstr = NULL;
	if (status == 200) {
		int content_length;
		sscanf(res_data + chars_scanned, "Content-Length: %d\r\n\r\n", &content_length);
		retstr = malloc(content_length + 1);
		read(conn->fd, retstr, content_length);
		retstr[content_length] = '\0';
	} else if (status == 201) {
		sscanf(res_data + chars_scanned, "Content-Location: %ms\r\n\r\n", &retstr);
	}
	close(conn->fd);
	return retstr;
}


// Main //
int main() {

	// text user interface
	const char *tui =				   	
	"		  Welcome to RSP Online\n"
	"	  The online game of rock paper scissors\n"
	"\n"
	"	 [ You are matched with a random person  ]\n"
	"	 [ Enter a number based on the 3 choices ]\n"
	"	 [            See if you win!            ]\n"
	"\n"
	"\n"
	"		| Type \"1\" for ROCK     |\n"
	"		| Type \"2\" for PAPER    |\n"
	"		| Type \"3\" for SCISSORS |\n";

	srand(time(NULL));
    puts(tui);
    puts("Connecting to Server...");
    struct connection_state conn;

	// client socket address
	conn.sa = (struct sockaddr_in) {
	    .sin_family = AF_INET,
	    .sin_port = htons(8080),
	    .sin_addr.s_addr = htonl(0x7f000001) // localhost
	};

	// Client prompts 
    puts("Added to Queue! Please wait");
    send_request(&conn, REQ_PUT, "/new");
    char *player_address = recv_response(&conn);
    if (!player_address) {
    	fputs("Invaild Address :(\n", stderr);
    	return 1;
    }
    puts("Game found!");
    int move;
	printf("Enter your move: ");
	scanf("%d", &move);
	printf("sent move %d\n", move);
    char move_uri[20];
    snprintf(move_uri, sizeof(move_uri), "%s/move/%1d", player_address, move);
    send_request(&conn, REQ_GET, move_uri);
	char *response = recv_response(&conn);
	puts(response);
    free(player_address);
    free(response);
	close(conn.fd);
}
