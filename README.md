#TicTacToo Cool
A game of TicTacToe using a client and a server, written in C. I've used Unix Pipes to implement the Client-Server game. 

The code is separated out into the client, t3client, that will act as the interface to the server, t3server, that will implement the game logic and track the state of the game. The client communicates with the server by writing to a pipe called serverpipe and reading from a pipe called clientpipe. The server communicates with the client by writing to a pipe called clientpipe and reading from a pipe called serverpipe.
