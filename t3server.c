#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "tictactoe.h"

typedef struct TicTacToe
{
  int size;      // this is the size of the game board
  int **board;   // this is the game board
  int winner;    // who won the game
} TicTacToe;

int check(TicTacToe* game);
void init_game(TicTacToe* game, int size);
void free_game(TicTacToe *game);
int get_player_move(int serverfd, int clientfd, TicTacToe* game);
void computer_move(TicTacToe* game);
void print_game(int clientfd, TicTacToe game);
char tokenstr(int token);
void print_result(int clientfd, TicTacToe game);
void client_continue(int clientfd);

int main(void)
{
	// define our variables related to pipes
	int clientfd, serverfd;
	char *clientpipe = "clientpipe";
	char *serverpipe = "serverpipe";

	// create the FIFO (named pipe) and open for reading 
	mkfifo(serverpipe, 0666);
	mkfifo(clientpipe, 0666);
	serverfd = open(serverpipe, O_RDONLY); // client talks to server
	clientfd = open(clientpipe, O_WRONLY); // server talks to client

	printf("This is the game of Tic Tac Toe.\n");
	printf("You will be playing against the computer.\n");

	int size;
	read(serverfd, &size, sizeof(size));

	// wait for a client to tell us how large is the game board
	printf("The game board is %d by %d.\n",size, size);

	// initialise the board
	TicTacToe game;   
	init_game(&game,size); 

	// as for the generic solution with addition of pipes
	
	int done;
	char command;
	do{
		read(serverfd, &command, sizeof(command)); /* get command from client */
		if (command == 'p'){ /* print command */
			print_game(clientfd, game);
		} else if (command == 'm'){ /* move command */
			do {
				done = get_player_move(serverfd, clientfd, &game);
				write(clientfd, &done, sizeof(done));
			} while (!done);
			
			int game_over = check(&game);
			write(clientfd, &game_over, sizeof(game_over));
			
			if (game_over == FALSE){
				computer_move(&game);
				game_over = check(&game);
				write(clientfd, &game_over, sizeof(game_over));
			}
		} else if (command == '0') { /* stop looping command */
			break;
		}
	} while(TRUE);
    
    print_game(clientfd, game);
    print_result(clientfd, game);

	// clean up
	close(serverfd);
	unlink(serverpipe);
	free_game(&game);
	return 0;
}

/* Initialize the matrix. */
void init_game(TicTacToe* game,int size)
{
	// set the size of the board
	game->size = size;
	
	// set winner to undecided
	game->winner = NONE;
	
	// use malloc to dynamically allocate 2D array  
	int **board = (int **)malloc(sizeof(int *) * size);
	
	int x;
    for (x = 0; x < size; x++) {
        board[x] = (int *)malloc(sizeof(int) * size);
    }
	
	// now initialise it
	int i, j;
    for(i=0; i<size; i++){
		for(j=0; j<size; j++){
			/* set to empty of tokens  */
			board[i][j] = NONE;
		}
	}
    
    game->board = board;
}

/* Deallocate the dynamically allocated memory */
void free_game(TicTacToe* game)
{
	// code goes here, as for your generic solution
	int i, size = game->size;

	for (i = 0; i < size; i++) {
		free(game->board[i]); /* free the 'buckets' holding the ints */
	}

	free(game->board); /* free board space */
}

/* Get a player's move. */ 
int get_player_move(int serverfd, int clientfd, TicTacToe* game)
{
	// read move from client one int at a time
	int x, y;

	read(serverfd, &x, sizeof(x));
	read(serverfd, &y, sizeof(y));

	// check if valid move or not
	int valid;
	
	if(game->board[x][y] != NONE) {
		printf("Invalid move, try again.\n");
		valid = FALSE;
	} else {
		game->board[x][y] = HUMAN;
		valid = TRUE;
	}
	
	// return result to client
	return valid;
}

/* Get a move from the computer. */
/* Return true (not 0) if can move */
/* Return false () if cannot move */
void computer_move(TicTacToe* game)
{
	// as for your generic solution
	int done = FALSE;
    int i,j,cx,cy;
    cx = cy = -1;
    for(i=0; i<game->size; i++){
        for(j=0; j<game->size; j++)
        if(game->board[i][j]==NONE){
            cx = i; cy = j;
            break;
        }
        if (cx != -1) {
            game->board[cx][cy] = COMPUTER;
            break;
        }
    }
}

/* Map the board token ID into a character */
char tokenstr(int t) 
{
  if(t==HUMAN) 
    return 'X';
  else if (t==COMPUTER)
    return 'O';
  return '.';
}

/* Send the game result to the client */
/* Do it a character at a time -- easy! */
void print_game(int clientfd, TicTacToe game)
{
	// write out the board one character at a time
	int i,j;
    
    /* read and print the board one character at a time */
    for(i=0; i<game.size; i++){
        for(j=0; j<game.size; j++){
			int symbol = game.board[i][j];
			char token = tokenstr(symbol);
            printf("%c",token);
			write(clientfd, &symbol, sizeof(symbol));
        }
        printf("\n");
		int new_line = 10; /* 10 represents a \n in the client */
		write(clientfd, &new_line, sizeof(new_line));
    }
    printf("\n");
	client_continue(clientfd);
}

/* See if there is a winner. */
/* return true (0) if so otherwise false */
int check(TicTacToe* game)
{
	// code is the same as your generic version
	int i,j;
    int count;
	int x, y;
	int x_total, y_total;
	int diagonal;
	
	/* checks rows */
	for (y = 0; y < game->size; y++) {
		x_total = 0;
		for (x = 0; x < game->size; x++) {
			x_total += game->board[y][x];
		}
		
		/* if x_total is positive or negative game size, someone has won */
		if (x_total == game->size || x_total == (game->size * -1)) {
			if (x_total < 0) {
				game->winner = HUMAN;
			} else {
				game->winner = COMPUTER;
			}
			return TRUE;
		}
	}
	
	/* checks columns */
	for (y = 0; y < game->size; y++) {
		y_total = 0;
		for (x = 0; x < game->size; x++) {
			y_total += game->board[x][y];
		}
		
		if (y_total == game->size || y_total == (game->size * -1)) {
			if (y_total < 0) {
				game->winner = HUMAN;
			} else {
				game->winner = COMPUTER;
			}
			return TRUE;
		}
	}
	
	/* top left to bottom right diagonal */
	diagonal = 0;
	for (x = 0; x < game->size; x++) {
		diagonal += game->board[x][x];
	}
	
	if (diagonal == game->size || diagonal == (game->size * -1)) {
		if (diagonal < 0) {
			game->winner = HUMAN;
		} else {
			game->winner = COMPUTER;
		}
		return TRUE;
	}
	
	/* bottom left to top right diagonal */
	y = game->size - 1;
	diagonal = 0;
	for (x = 0; x < game->size; x++) {
		diagonal += game->board[x][y];
		y--;
	}
	
	if (diagonal == game->size || diagonal == (game->size * -1)) {
		if (diagonal < 0) {
			game->winner = HUMAN;
		} else {
			game->winner = COMPUTER;
		}
		return TRUE;
	}
    
    /* test if out of space on the board */
    count=0;
    for(i=0; i<game->size; i++){
        for(j=0; j<game->size; j++) {
            if(game->board[i][j]==NONE) count++;
        }
    }
    if(count==0) {
        game->winner=DRAW;
        return TRUE;
    }
    
    /* no-one and nor was there a draw */
    return FALSE;
}

/* Tell the client that game has ended and the result of the game */
void print_result(int clientfd, TicTacToe game) 
{
	// send game over
	int game_over = 0;
	write(clientfd, &game_over, sizeof(game_over));
	
	// tell the client the winner
	int winner = game.winner;
	write(clientfd, &winner, sizeof(winner));
}

/* Tell the client to continue */
void client_continue(int clientfd) 
{
	// tell the client that it should continue playing
	int cont = 11; /* 11 represents continue in the client */
	write(clientfd, &cont, sizeof(cont));
}
