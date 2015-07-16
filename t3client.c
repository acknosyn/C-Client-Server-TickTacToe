#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "tictactoe.h"

int init_game(int serverfd);
int check(int clientfd);
int player_move(int clientfd, int serverfd);
void print_game(int clientfd, int size);
void print_winner(int clientfd);

// implement a very simple client that sends and receives
// all other logic, just send and receive strings
// extension is to add extra clients
int main(void)
{
	// define our variables related to pipes
	int clientfd, serverfd;
	char *clientpipe = "clientpipe";
	char *serverpipe = "serverpipe";

	// create the FIFO (named pipe) and open for reading 
	mkfifo(clientpipe, 0666);
	mkfifo(serverpipe, 0666);
	serverfd = open(serverpipe, O_WRONLY);
	clientfd = open(clientpipe, O_RDONLY);

	printf("This is the game of Tic Tac Toe.\n");
	printf("You will be playing against the computer.\n");

	int done;  // used to check validity of move
	int game_over; // used to check if game has completed

	// client must send commands and wait for responses
	// client exits loop if game is over
	// client should continue if receives continue message

	int size = init_game(serverfd); /* send the server the size of the game board */
	
	/* loop until the server tells us the game is over */	
	do{
		char command = 'p'; /* tell server to print game */
		write(serverfd, &command, sizeof(command));
		
		print_game(clientfd, size);
		do {
			command = 'm'; /* tell server to look for move */
			write(serverfd, &command, sizeof(command));
			
			done = player_move(clientfd, serverfd);
		}while(!done); /* loop until valid move */
		
		/* check to see if either player or computer has won */
		if (check(clientfd) != FALSE || check(clientfd) != FALSE){
			command = '0'; /* tell server to stop looping */
			write(serverfd, &command, sizeof(command));
			break; /* was a win or draw? */
		}

	} while (TRUE);

	char command = 'p'; /* tell server to print game one last time */
	write(serverfd, &command, sizeof(command));
	
	/* display winner */
	print_game(clientfd, size); /* show final positions */
	print_winner(clientfd);
	
	// tidy up
	close(clientfd);
	unlink(clientpipe);

	return 0;
}

/* Start the game by telling server size of the board */
int init_game(int serverfd) 
{
	int size; /* size of game board */
	
    // ask the player
	printf("Enter the size of the board: ");
	scanf("%d", &size);
	
    // tell the server
	write(serverfd, &size, sizeof(size)); /* send size of game board to server */
  
    return size;
}

/* Send player's move to the server and check if was valid. */
int player_move(int clientfd, int serverfd)
{
	int x, y;
	int valid = FALSE;
	
	// ask the player
	do {
	printf("Enter the first coordinate (row) for your move: ");
	scanf("%d", &x);
	printf("Enter the second coordinate (column) for your move: ");
	scanf("%d", &y);
	
	x--; y--;
	
	// send the details
	write(serverfd, &x, sizeof(x));
	write(serverfd, &y, sizeof(y));
	
	// check the result
	read(clientfd, &valid, sizeof(valid));
	} while (valid == 0);
	
	return valid;
}

/* Display the game on the screen. */
void print_game(int clientfd, int size)
{
	int printing = TRUE; /* keep printing until server says to stop */
	
    // read and print the board one character at a time
    while (printing) {
		int symbol;
		read(clientfd, &symbol, sizeof(symbol));
		
		if (symbol == 11){
		  printing = FALSE; /* stop printing */
		} else {
			if (symbol == 10){ /* new_line symbol */
				printf("\n");
			} else if (symbol == NONE){
				printf(".");
			} else if (symbol == COMPUTER) {
				printf("O");
			} else if (symbol == HUMAN){
				printf("X");
			}
		}
	} 
}

/* Check whether game has ended or whether computer is waiting 
   for the next move */
int check(int clientfd) 
{
	// what does the server tell us? do we continue?
	// someone did win, so the game is over
	int check = 0;
	read(clientfd, &check, sizeof(check));

	return check;
}

/* Receive the final results from the server */
void print_winner(int clientfd)
{
	int game_over;
	read(clientfd, &game_over, sizeof(game_over));
	
	if (game_over == 0){
		int winner;
		read(clientfd, &winner, sizeof(winner));

		if(winner==HUMAN) printf("You won!\n");
		else if(winner==COMPUTER) printf("I won!!!!\n");
		else printf("Draw :(\n");
	}
   
}

