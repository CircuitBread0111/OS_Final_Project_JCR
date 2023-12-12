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
 * Plays a simple game of
 * Rock Paper Scissors Online!=
 * With other clients!
 */

//----------------------------------------------------------------

// Includes //
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
//
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//
#include "queue.h"

// Defines //
#define MAX_PLAYER_COUNT 4096
#define PLAYER_WAITING ((void *) 1)

// Game States //
struct game_state {
    int players[2];
    int moves[2]; 
};

// sets players to 0 //
static struct game_state *players[MAX_PLAYER_COUNT] = {0};

// Game outcomes //
enum winner { A_WIN, B_WIN, DRAW };

// Posible moves //
enum game_move { MOVE_NONE, MOVE_ROCK, MOVE_PAPER, MOVE_SCISSORS }; 



//	GAME LOGIC  //
///////////////////////////////////////////////////////////
int game_logic(int a, int b) {

    // if users pick same
    if (a == b) {
        return DRAW;
    }

    //Game Logic 1  (a = 1)
    else if (a == MOVE_ROCK && b == MOVE_PAPER) {
        return B_WIN;
    }
    else if (a == MOVE_ROCK && b == MOVE_SCISSORS) {
        return A_WIN;
    }

    //Game logic 2  (a = 2)
    else if (a == MOVE_PAPER && b == MOVE_ROCK) {
        return A_WIN;
    }
    else if (a == MOVE_PAPER && b == MOVE_SCISSORS) {
        return B_WIN;
    }

     //Game logic 3  (a = 3)
    else if (a == MOVE_SCISSORS && b == MOVE_ROCK) {
        return B_WIN;
    }
    else if (a == MOVE_SCISSORS && b == MOVE_PAPER) {
        return A_WIN;
    }

    __builtin_unreachable();
}
///////////////////////////////////////////////////////////




static Queue *player_queue; 									// Player Queue
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER; // PThread mutex

// Connection data //
struct connection_data {
    volatile int client_fd; // Client file descriptor
    volatile int id; 		// Client id
};

// Creates Game //
// param: player 1 and player 2
static void create_game(int p1, int p2) {
	struct game_state *game = calloc(1, sizeof(*game));
	game->players[0] = p1;
	game->players[1] = p2;
	players[p1] = game;
	players[p2] = game;
}


// Adds Players to Waiting Queue //
static int add_player(void) {

	pthread_mutex_lock(&queue_mutex);
	int new_player_id; 
	// creates new unique id for each player
	do {
		new_player_id = rand() & (MAX_PLAYER_COUNT - 1);
	} while (players[new_player_id]);
	players[new_player_id] = PLAYER_WAITING;
	// Pairs clients to game
	if (Queue_Size(player_queue) > 0) {
		int queued_player_id;
		Queue_Dequeue(player_queue, &queued_player_id);
		printf("creating game with %03x and %03x\n", queued_player_id, new_player_id);
		create_game(queued_player_id, new_player_id);
	} else {
		Queue_Enqueue(player_queue, new_player_id); 
	}
	pthread_mutex_unlock(&queue_mutex);
	return new_player_id;
}


// Sets the Players move //
// param: player and their selected move
static int player_move(int player, int move) {
	struct game_state *game = players[player];
	int which_player = player == game->players[1];
	game->moves[which_player] = move;
	while (!game->moves[1 - which_player]) sched_yield();
	// returns outcome
	return game_logic(game->moves[which_player], game->moves[1 - which_player]);
}

// Removes mutex
static pthread_mutex_t removal_mutex = PTHREAD_MUTEX_INITIALIZER;

// Send results //
// param: file descriptor and winner
static void send_result(int fd, int winner) {
	char *result_text[] = { "You win!", "You lose.", "Draw!" };
	dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Length: %zd\r\n\r\n%s", strlen(result_text[winner]), result_text[winner]);
}


// Remove game //
// param: player id //
static void remove_game(int player_id) {
	pthread_mutex_lock(&removal_mutex);
	if (players[player_id]) {
		int *game_players = players[player_id]->players;
		struct game_state *removed_game = players[game_players[0]];
		players[game_players[0]] = NULL;
		players[game_players[1]] = NULL;
		free(removed_game);
	}
	pthread_mutex_unlock(&removal_mutex);
}


// Connection Thread //
// param: void pointer to arg
// arg MUST BE malloced!
static void *connection_thread(void *arg) { 

    struct connection_data *cdata = arg;	// sets connection data to arg
    char req_data[8192]; 					// allowed requested data amount of 8192 characters
    ssize_t req_size = 0;					// size of requested data set to 0
    char *uri = NULL;						// client uri
	int player_id; 							// client's player id
	int move;								// players move choice
	
    // checks requested data
	do {
    	req_size += read(cdata->client_fd, req_data + req_size, 1);
    	fputc(req_data[req_size - 1], stderr); 
	} while (req_size < 4 || (
		req_data[req_size - 4] != '\r' ||
    	req_data[req_size - 3] != '\n' ||
    	req_data[req_size - 2] != '\r' ||
    	req_data[req_size - 1] != '\n'
    ));
	req_data[req_size] = '\0';
	
	// HTTP Request. I will assume you are doing GET or PUT according to what the URI is.
	if (sscanf(req_data, "%*s %ms HTTP/1.1\r\n\r\n", &uri) == 1) {
	
		// Client notification to server
		if (!strcmp(uri, "/new")) {
			player_id = add_player();
			dprintf(cdata->client_fd, "HTTP/1.1 201 Created\r\nContent-Location: /player/%03x\r\n\r\n", player_id);
			printf("%x added to queue\n", player_id);
				
		// move is stored in the URI
		} else if (sscanf(uri, "/player/%03x/move/%1d", &player_id, &move) == 2) {
			while ((void *) players[player_id] < (void *) 2) sched_yield(); // spin-lock
			int winner = player_move(player_id, move);
			send_result(cdata->client_fd, winner);
			remove_game(player_id);
		} else {
			dprintf(cdata->client_fd, "HTTP/1.1 404 Not Found\r\n\r\n");
			printf("404 not found\n"); // HTTP Error Code
		}
	} else {
		dprintf(cdata->client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n");
		printf("bad request\n"); // HTTP Error Code
	}
	if (uri) free(uri);
	close(cdata->client_fd);
    free(cdata);
    return NULL;
}


// Main //
int main(void) {
	
	// Creates socket
    int server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return 1;
    }
    
	// Socket address
    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port = htons(8080), // localhost
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    
	// Bind Socket
    if (bind(server_fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        return 1;
    }
    
	// Socket lisener
    if (listen(server_fd, 128) == -1) {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        return 1;
    }
    
	// Creates player queue for clients
	player_queue = Queue_Create(1024);
	puts("Server Ready!"); // Notifies that the server is up
    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd != -1) {
            pthread_t pt;
            struct connection_data *cdata = malloc(sizeof(*cdata));
            cdata->client_fd = client_fd;
            pthread_create(&pt, NULL, &connection_thread, cdata);
        }
    }
    // Closes server
    close(server_fd);
}
