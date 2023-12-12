// A game of rps

#include<stdio.h>
#include<stdlib.h>

int main() {
//TUI
	const char *tui =				   	
	"Welcome to RSP Online                                 \n"
	"-----------------------------                         \n"
	"The online game of rock paper scissors                \n"
	"													   \n"
	"													   \n"
	"(How to Play)                          			   \n"
	"[ You are matched with a random person         ]      \n"
	"[ Enter a numuserBer userBased on the 3 choices]      \n"
	"[              See if you win!                 ]      \n"
	"													   \n"		
	"													   \n"
	"| Type \"1\" for ROCK     |            			   \n"
	"| Type \"2\" for PAPER    |		        		   \n"
	"| Type \"3\" for SCISSORS |              	 		   \n";

	puts(tui);
	
	int userA, userB;
	char request[2];
	printf(": ");
	scanf("%d %d", &userA, &userB);


	//	GAME LOGIC  //
///////////////////////////////////////////////////////////

    // if users pick same
    if (userA == userB) {
        printf("Draw");
    }

    //Game Logic 1  (a = 1)
    else if (userA == 1 && userB == 2) {
        printf("userB Win");
    }
    else if (userA == 1 && userB == 3) {
        printf("userA Win");
    }

    //Game logic 2  (a = 2)
    else if (userA == 2 && userB == 1) {
        printf("userA Win");
    }
    else if (userA == 2 && userB == 3) {
        printf("userB Win");
    }

     //Game logic 3  (a = 3)
    else if (userA == 3 && userB == 1) {
        printf("userB Win");
    }
    else if (userA == 3 && userB == 2) {
        printf("userA Win");
    }

   
///////////////////////////////////////////////////////////

	
	
}
