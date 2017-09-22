#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#include <iostream>
#include "cli.h"
#include "protos.h"
#include "extglobals.h"
#include "board.h"

void CLI::readCommands()
{
	int nextc;

	if (board.nextMove == WHITE_MOVE)
	{
		std::cout << "wt> ";
	}
	else
	{
		std::cout << "bl> ";
	}
	std::cout.flush();

	//     ===========================================================================
	//     Read a command and call doCommand:
	//     ===========================================================================
	while ((nextc = getc(stdin)) != EOF)
	{
		if (nextc == '\n')
		{
			CMD_BUFF[CMD_BUFF_COUNT] = '\0';
			while (CMD_BUFF_COUNT)
			{
				if (!doCommand(CMD_BUFF)) return;
			}
			if (board.nextMove == WHITE_MOVE)
			{
				std::cout << "wt> ";
			}
			else
			{
				std::cout << "bl> ";
			}
			std::cout.flush();
		}
		else
		{
			if (CMD_BUFF_COUNT >= MAX_CMD_BUFF - 1)
			{
				std::cout << "Warning: command buffer full !! " << std::endl;
				CMD_BUFF_COUNT = 0;
			}
			CMD_BUFF[CMD_BUFF_COUNT++] = nextc;
		}
	}
}

bool CLI::doCommand(const char *buf)
{

	char userinput[80];
	int number;

	//     =================================================================
	//  return when command buffer is empty
	//     =================================================================

	if (!strcmp(buf, ""))
	{
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  help, h, or ?: show this help
	//     =================================================================
	if ((!strcmp(buf, "help")) || (!strcmp(buf, "h")) || (!strcmp(buf, "?")))
	{
		std::cout << std::endl << "help:" << std::endl;
		std::cout << "black               : BLACK to move" << std::endl;
		std::cout << "cc                  : play computer-to-computer " << std::endl;
		std::cout << "d                   : display board " << std::endl;
		std::cout << "exit                : exit program " << std::endl;
		std::cout << "eval                : show static evaluation of this position" << std::endl;
		std::cout << "game                : show game moves " << std::endl;
		std::cout << "go                  : computer next move " << std::endl;
		std::cout << "help, h, or ?       : show this help " << std::endl;
		std::cout << "info                : display variables (for testing purposes)" << std::endl;
		std::cout << "move e2e4, or h7h8q : enter a move (use this format)" << std::endl;
		std::cout << "moves               : show all legal moves" << std::endl;
		std::cout << "new                 : start new game" << std::endl;
		std::cout << "perf                : benchmark a number of key functions" << std::endl;
		std::cout << "perft n             : calculate raw number of nodes from here, depth n " << std::endl;
		std::cout << "quit                : exit program " << std::endl;
		std::cout << "r                   : rotate board " << std::endl;
		std::cout << "readfen filename n  : reads #-th FEN position from filename" << std::endl;
		std::cout << "sd n                : set the search depth to n" << std::endl;
		std::cout << "setup               : setup board... " << std::endl;
		std::cout << "undo                : take back last move" << std::endl;
		std::cout << "white               : WHITE to move" << std::endl;
		std::cout << std::endl;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  black: black to move
	//     =================================================================
	if (!strcmp(buf, "black"))
	{
		board.nextMove = BLACK_MOVE;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  d: display board
	//     =================================================================
	if (!strcmp(buf, "d"))
	{
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  exit or quit: exit program
	//     =================================================================
	if ((!strcmp(buf, "exit")) || (!strcmp(buf, "quit")))
	{
		CMD_BUFF_COUNT = '\0';
		return false;
	}

	//     =================================================================
	//  info: display variables (for testing purposes)
	//     =================================================================
	if (!strcmp(buf, "info"))
	{
		info();
		CMD_BUFF_COUNT = '\0';
		return true;
	}


	//     =================================================================
	//  new: start new game
	//     =================================================================
	if (!strcmp(buf, "new"))
	{
		dataInit();
		board.init();
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  r: rotate board
	//     =================================================================
	if (!strcmp(buf, "r"))
	{
		board.viewRotated = !board.viewRotated;
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  readfen filename n : reads #-th FEN position from filename
	//     =================================================================
	if (!strncmp(buf, "readfen", 7))
	{
		sscanf(buf + 7, "%s %d", userinput, &number);
		board.init();
		readFen(userinput, number);
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  white: white to move
	//     =================================================================
	if (!strcmp(buf, "white"))
	{
		board.nextMove = WHITE_MOVE;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  setup               : setup board...
	//     =================================================================
	if (!strncmp(buf, "setup", 5))
	{
		setup();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  unknown command
	//     =================================================================
	std::cout << "    command not implemented: " << buf << ", type 'help' for more info" << std::endl;
	CMD_BUFF_COUNT = '\0';
	return true;
}

bool FEN::readFen(char *filename, int number)
{
	int numberf;
	char s[180];
	char fenwhite[80];
	char fenblack[80];
	char fen[100];
	char fencolor[2];
	char fencastling[5];
	char fenenpassant[3];
	char temp[80];
	int fenhalfmoveclock;
	int fenfullmovenumber;
	bool returnValue;
	FILE * fp;

	returnValue = false;
	if (number <= 0) return returnValue;

	// open the file for read and scan through until we find the number-th position:
	fp = fopen(filename, "rt");
	if (fp != NULL)
	{
		numberf = 0;
		while (fscanf(fp, "%s", s) != EOF)
		{
			if (!strcmp(s, "[White"))
			{
				fscanf(fp, "%s", fenwhite);
				// remove first (") and last two characters ("]) from fenwhite:
				strcpy(temp, "");
				strncat(temp, fenwhite, strlen(fenwhite) - 2);
				strcpy(temp, temp + 1);
				strcpy(fenwhite, temp);
			}
			if (!strcmp(s, "[Black"))
			{
				fscanf(fp, "%s", fenblack);
				// remove first (") and last two characters ("]) from fenblack:
				strcpy(temp, "");
				strncat(temp, fenblack, strlen(fenblack) - 2);
				strcpy(temp, temp + 1);
				strcpy(fenblack, temp);
			}
			if (!strcmp(s, "[FEN"))
			{
				// position found, so increment numberf.
				// we already have fenwhite and fenblack.
				numberf++;
				if (numberf == number)
				{
					fscanf(fp, "%s", fen);
					fscanf(fp, "%s", fencolor);           // b or w
					fscanf(fp, "%s", fencastling);        // -, or KQkq
					fscanf(fp, "%s", fenenpassant);       // -, or e3, or b6, etc
					fscanf(fp, "%d", &fenhalfmoveclock);  // int, used for the fifty move draw rule
					fscanf(fp, "%d", &fenfullmovenumber); // int. start with 1, It is incremented after move by Black

					std::cout << std::endl << "winglet> fen #" << numberf << " in " << filename << ":" << std::endl << std::endl;
					std::cout << " White: " << fenwhite << std::endl;
					std::cout << " Black: " << fenblack << std::endl;
					std::cout << " " << &fen[1] << std::endl;
					if (fencolor[0] == 'w')
					{
						std::cout << " wt to move next" << std::endl;
					}
					else
					{
						std::cout << " bl to move next" << std::endl;
					}
					std::cout << " Castling: " << fencastling << std::endl;
					std::cout << " EP square: " << fenenpassant << std::endl;
					std::cout << " Fifty move count: " << fenhalfmoveclock << std::endl;
					std::cout << " Move number: " << fenfullmovenumber << std::endl << std::endl;
				}
			}
		}

		if (numberf < number)
		{
			printf("winglet> only %d fens present in %s, fen #%d not found\n",
				numberf, filename, number);
			returnValue = false;
		}
		else
		{
			setupFen(fen, fencolor, fencastling, fenenpassant, fenhalfmoveclock, fenfullmovenumber);
			returnValue = true;
		}
		fclose(fp);
	}
	else
	{
		printf("winglet> error opening file: %s\n", filename);
		returnValue = false;
	}
	return returnValue;
}

void FEN::setupFen(char *fen, char *fencolor, char *fencastling, char *fenenpassant, int fenhalfmoveclock, int fenfullmovenumber)
{
	int i, file, rank, counter, piece;
	int whiteCastle, blackCastle, next, epsq;

	piece = 0;
	for (i = 0; i < 64; i++)
	{
		board.square[i] = EMPTY;
	}

	// loop over the FEN string characters, and populate board.square[]
	// i is used as index for the FEN string
	// counter is the index for board.square[], 0..63
	// file and rank relate to the position on the chess board, 1..8
	// There is no error/legality checking on the FEN string!!
	file = 1;
	rank = 8;
	i = 0;
	counter = 0;
	while ((counter < 64) && (fen[i] != '\0'))
	{
		// '1'  through '8':
		if (((int)fen[i] > 48) && ((int)fen[i] < 57))
		{
			file += (int)fen[i] - 48;
			counter += (int)fen[i] - 48;
		}
		else
			//  other characters:
		{
			switch (fen[i])
			{
			case '/':
				rank--;
				file = 1;
				break;

			case 'P':
				board.square[BOARDINDEX[file][rank]] = WHITE_PAWN;
				file += 1;
				counter += 1;
				break;

			case 'N':
				board.square[BOARDINDEX[file][rank]] = WHITE_KNIGHT;
				file += 1;
				counter += 1;
				break;

			case 'B':
				board.square[BOARDINDEX[file][rank]] = WHITE_BISHOP;
				file += 1;
				counter += 1;
				break;

			case 'R':
				board.square[BOARDINDEX[file][rank]] = WHITE_ROOK;
				file += 1;
				counter += 1;
				break;

			case 'Q':
				board.square[BOARDINDEX[file][rank]] = WHITE_QUEEN;
				file += 1;
				counter += 1;
				break;

			case 'K':
				board.square[BOARDINDEX[file][rank]] = WHITE_KING;
				file += 1;
				counter += 1;
				break;

			case 'p':
				board.square[BOARDINDEX[file][rank]] = BLACK_PAWN;
				file += 1;
				counter += 1;
				break;

			case 'n':
				board.square[BOARDINDEX[file][rank]] = BLACK_KNIGHT;
				file += 1;
				counter += 1;
				break;

			case 'b':
				board.square[BOARDINDEX[file][rank]] = BLACK_BISHOP;
				file += 1;
				counter += 1;
				break;

			case 'r':
				board.square[BOARDINDEX[file][rank]] = BLACK_ROOK;
				file += 1;
				counter += 1;
				break;

			case 'q':
				board.square[BOARDINDEX[file][rank]] = BLACK_QUEEN;
				file += 1;
				counter += 1;
				break;

			case 'k':
				board.square[BOARDINDEX[file][rank]] = BLACK_KING;
				file += 1;
				counter += 1;
				break;

			default:
				break;
			}
		}
		i++;
	}
	next = WHITE_MOVE;
	if (fencolor[0] == 'b') next = BLACK_MOVE;

	whiteCastle = 0;
	blackCastle = 0;
	if (strstr(fencastling, "K")) whiteCastle += CANCASTLEOO;
	if (strstr(fencastling, "Q")) whiteCastle += CANCASTLEOOO;
	if (strstr(fencastling, "k")) blackCastle += CANCASTLEOO;
	if (strstr(fencastling, "q")) blackCastle += CANCASTLEOOO;
	if (strstr(fenenpassant, "-"))
	{
		epsq = 0;
	}
	else
	{
		// translate a square coordinate (as string) to int (eg 'e3' to 20):
		epsq = ((int)fenenpassant[0] - 96) + 8 * ((int)fenenpassant[1] - 48) - 9;
	}
	board.initFromSquares(board.square, next, fenhalfmoveclock, whiteCastle, blackCastle, epsq);

}