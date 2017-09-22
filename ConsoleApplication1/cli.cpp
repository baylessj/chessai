#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <iostream>
#include "cli.h"
#include "board.h"
#include "pieces.h"

using namespace std;

void CLI::readCommands()
{
	int nextc;

	if (board.nextMove == WHITE_MOVE)
	{
		cout << "wt> ";
	}
	else
	{
		cout << "bl> ";
	}
	cout.flush();

	// Read a command
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
				cout << "wt> ";
			}
			else
			{
				cout << "bl> ";
			}
			cout.flush();
		}
		else
		{
			if (CMD_BUFF_COUNT >= MAX_CMD_BUFF - 1)
			{
				cout << "Warning: command buffer full !! " << endl;
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

	if (!strcmp(buf, ""))
	{
		CMD_BUFF_COUNT = '\0';
		return true; // return when buffer is empty
	}

	// Help Menu
	if ((!strcmp(buf, "help")) || (!strcmp(buf, "h")) || (!strcmp(buf, "?")))
	{
		cout << endl << "help:" << endl;
		cout << "black               : BLACK to move" << endl;
		cout << "cc                  : play computer-to-computer " << endl;
		cout << "d                   : display board " << endl;
		cout << "exit                : exit program " << endl;
		cout << "eval                : show static evaluation of this position" << endl;
		cout << "game                : show game moves " << endl;
		cout << "go                  : computer next move " << endl;
		cout << "help, h, or ?       : show this help " << endl;
		cout << "info                : display variables (for testing purposes)" << endl;
		cout << "move e2e4, or h7h8q : enter a move (use this format)" << endl;
		cout << "moves               : show all legal moves" << endl;
		cout << "new                 : start new game" << endl;
		cout << "perf                : benchmark a number of key functions" << endl;
		cout << "perft n             : calculate raw number of nodes from here, depth n " << endl;
		cout << "quit                : exit program " << endl;
		cout << "r                   : rotate board " << endl;
		cout << "readfen filename n  : reads #-th FEN position from filename" << endl;
		cout << "sd n                : set the search depth to n" << endl;
		cout << "setup               : setup board... " << endl;
		cout << "undo                : take back last move" << endl;
		cout << "white               : WHITE to move" << endl;
		cout << endl;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Black to Move
	if (!strcmp(buf, "black"))
	{
		board.nextMove = BLACK_MOVE;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Display Board
	if (!strcmp(buf, "d"))
	{
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Exit Program
	if ((!strcmp(buf, "exit")) || (!strcmp(buf, "quit")))
	{
		CMD_BUFF_COUNT = '\0';
		return false;
	}

	// Debug info
	if (!strcmp(buf, "info"))
	{
		info();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// New Game
	if (!strcmp(buf, "new"))
	{
		BITSET[0] = 0x1;
		for (int i = 1; i < 64; i++)
		{
			BITSET[i] = BITSET[i - 1] << 1;
		}

		//     ===========================================================================
		//     BOARDINDEX is used to translate [file][rank] to [square],
		//  Note that file is from 1..8 and rank from 1..8 (not starting from 0)
		//     ===========================================================================
		for (int rank = 0; rank < 9; rank++)
		{
			for (int file = 0; file < 9; file++)
			{
				BOARDINDEX[file][rank] = (rank - 1) * 8 + file - 1;
			}
		}

		board.init();

		//     ===========================================================================
		//     Initialize MS1BTABLE, used in lastOne (see bitops.cpp)
		//     ===========================================================================
		for (int i = 0; i < 256; i++)
		{
			MS1BTABLE[i] = (
				(i > 127) ? 7 :
				(i >  63) ? 6 :
				(i >  31) ? 5 :
				(i >  15) ? 4 :
				(i >   7) ? 3 :
				(i >   3) ? 2 :
				(i >   1) ? 1 : 0);
		}

		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Rotate Board
	if (!strcmp(buf, "r"))
	{
		board.viewRotated = !board.viewRotated;
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Reads #-th FEN position from filename
	if (!strncmp(buf, "readfen", 7))
	{
		sscanf(buf + 7, "%s %d", userinput, &number);
		board.init();
		readFen(userinput, number);
		board.display();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// White to Move
	if (!strcmp(buf, "white"))
	{
		board.nextMove = WHITE_MOVE;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Board Setup
	if (!strncmp(buf, "setup", 5))
	{
		setup();
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	// Unknown Command
	cout << "    command not implemented: " << buf << ", type 'help' for more info" << endl;
	CMD_BUFF_COUNT = '\0';
	return true;
}

CLI::CLI() {
	// BITSET has only one bitset
	BITSET[0] = 0x1;
	for (int i = 1; i < 64; i++)
	{
		BITSET[i] = BITSET[i - 1] << 1;
	}

	//     ===========================================================================
	//     BOARDINDEX is used to translate [file][rank] to [square],
	//  Note that file is from 1..8 and rank from 1..8 (not starting from 0)
	//     ===========================================================================
	for (int rank = 0; rank < 9; rank++)
	{
		for (int file = 0; file < 9; file++)
		{
			BOARDINDEX[file][rank] = (rank - 1) * 8 + file - 1;
		}
	}

	// Initialize the Board
	board.init();

	//     ===========================================================================
	//     Initialize MS1BTABLE, used in lastOne (see bitops.cpp)
	//     ===========================================================================
	for (int i = 0; i < 256; i++)
	{
		MS1BTABLE[i] = (
			(i > 127) ? 7 :
			(i >  63) ? 6 :
			(i >  31) ? 5 :
			(i >  15) ? 4 :
			(i >   7) ? 3 :
			(i >   3) ? 2 :
			(i >   1) ? 1 : 0);
	}

	return;
}

void CLI::info()
{

	//  your playground... display variables - meant for testing/verification purposes only
	cout << endl << "============ info start ==============" << endl;
	cout << "size of board, in bytes   = " << sizeof(board) << endl;
	cout << "Material value            = " << board.Material << endl;
	cout << "White castling rights     = " << int(board.castleWhite) << endl;
	cout << "Black castling rights     = " << int(board.castleBlack) << endl;
	cout << "En-passant square         = " << board.epSquare << endl;
	cout << "Fifty move count          = " << board.fiftyMove << endl;

	cout << "bitCnt of white pawns     = " << bitCnt(board.whitePawns) << endl;
	cout << endl << "bitmap of blackKnights | board.whitePawns:" << endl;
	displayBitmap(board.blackKnights | board.whitePawns);
	cout << "============ info end ================" << endl << endl;

	return;
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

					cout << endl << "winglet> fen #" << numberf << " in " << filename << ":" << endl << endl;
					cout << " White: " << fenwhite << endl;
					cout << " Black: " << fenblack << endl;
					cout << " " << &fen[1] << endl;
					if (fencolor[0] == 'w')
					{
						cout << " wt to move next" << endl;
					}
					else
					{
						cout << " bl to move next" << endl;
					}
					cout << " Castling: " << fencastling << endl;
					cout << " EP square: " << fenenpassant << endl;
					cout << " Fifty move count: " << fenhalfmoveclock << endl;
					cout << " Move number: " << fenfullmovenumber << endl << endl;
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

void CLI::setup()
{
	// interactively setup the chess board
	int file, rank;
	int whiteCastle;
	int blackCastle;
	unsigned char next;
	int halfmoves;
	int epsq, i;
	char s[80], epsqc[80], castle[80], fenrank[80];
	char fen[100];
	char fencolor[2];
	char fencastling[5];
	char fenenpassant[3];
	int fenhalfmoveclock;
	int fenfullmovenumber;

	whiteCastle = board.castleWhite;
	blackCastle = board.castleBlack;
	next = board.nextMove;
	halfmoves = board.fiftyMove;
	epsq = board.epSquare;

	if (board.nextMove == WHITE_MOVE)
	{
		cout << "wt> setup> type 'help' for more info" << endl;
	}
	else
	{
		cout << "bl> setup> type 'help' for more info" << endl;
	}

	// infinite loop - user input:
	for (;;)
	{
		if (board.nextMove == WHITE_MOVE)
		{
			cout << "wt> setup> ";
		}
		else
		{
			cout << "bl> setup> ";
		}
		cout.flush();
		cin >> s;

		if ((!strcmp(s, "help")) || (!strcmp(s, "h")) || (!strcmp(s, "?")))
		{
			cout << endl << "setup help:" << endl;
			cout << "black               : BLACK to move" << endl;
			cout << "castle cccc         : castling rights, using FEN-style. Example: 'castle KQkq'" << endl;
			cout << "clear               : clear the board" << endl;
			cout << "d                   : display board" << endl;
			cout << "epsq cc             : set en-passant target square. Example: 'epsq e3'" << endl;
			cout << "exit                : exit setup" << endl;
			cout << "fen fenstring       : sets up the board with a FEN-string (6 elements)," << endl;
			cout << "                      for instance: n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1" << endl;
			cout << "                      you can paste a string into the windows console" << endl;
			cout << "                      by using your mouse and right-click paste" << endl;
			cout << "fifty n             : n = half moves since last capture or pawn move" << endl;
			cout << "new                 : new game" << endl;
			cout << "r                   : rotate board" << endl;
			cout << "rank n fenrank      : piece placement for rank n (from white's perspective)" << endl;
			cout << "                      fenrank defines the contents of each square, from left to" << endl;
			cout << "                      right (file a through file h). fenrank uses FEN-style:" << endl;
			cout << "                      pieces are identified by a single letter (pawn=P," << endl;
			cout << "                      knight=N, etc), using upper-case letters for white pieces" << endl;
			cout << "                      and lowercase letters for black pieces." << endl;
			cout << "                      Blank squares are noted using digits 1 through 8 (the " << endl;
			cout << "                      number of blank squares)." << endl;
			cout << "                      Examples: 'rank 1 R1BQKBNR' or 'rank 6 3p2p1'" << endl;
			cout << "white               : WHITE to move" << endl << endl;
		}

		else if (!strcmp(s, "black"))
		{
			next = BLACK_MOVE;
			board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
		}

		else if (!strncmp(s, "castle", 5))
		{
			cin >> castle;
			whiteCastle = 0;
			blackCastle = 0;
			if (strstr(castle, "K")) whiteCastle += CANCASTLEOO;
			if (strstr(castle, "Q")) whiteCastle += CANCASTLEOOO;
			if (strstr(castle, "k")) blackCastle += CANCASTLEOO;
			if (strstr(castle, "q")) blackCastle += CANCASTLEOOO;
			board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
		}

		else if (!strcmp(s, "clear"))
		{
			for (i = 0; i < 64; i++)
			{
				board.square[i] = EMPTY;
			}
			next = WHITE_MOVE;
			halfmoves = 0;
			whiteCastle = 0;
			blackCastle = 0;
			epsq = 0;
			board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
		}

		else if (!strcmp(s, "d"))
		{
			board.display();
			cout << "  castleWhite = " << (int)board.castleWhite << " castleBlack = " << (int)board.castleBlack << " epSquare = "
				<< board.epSquare << " fiftyMove = " << board.fiftyMove << endl << endl;
		}

		else if (!strncmp(s, "epsq", 4))
		{
			cin >> epsqc;
			epsq = ((int)epsqc[0] - 96) + 8 * ((int)epsqc[1] - 48) - 9;
			board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
		}

		else if (!strcmp(s, "exit"))
		{
			cout.flush();
			cin.clear();
			return;
		}

		else if (!strncmp(s, "fen", 3))
		{
			cin >> fen;
			cin >> fencolor;
			cin >> fencastling;
			cin >> fenenpassant;
			cin >> fenhalfmoveclock;
			cin >> fenfullmovenumber;
			FEN::setupFen(fen, fencolor, fencastling, fenenpassant, fenhalfmoveclock, fenfullmovenumber);
		}

		else if (!strncmp(s, "fifty", 5))
		{
			cin >> halfmoves;
			board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
		}

		else if (!strcmp(s, "new"))
		{
			board.init();
		}

		else if (!strcmp(s, "rank"))
		{
			cin >> rank;
			if ((rank > 0) & (rank < 9))
			{
				cin >> fenrank;
				// clear the file
				for (file = 1; file < 9; file++)
				{
					board.square[BOARDINDEX[file][rank]] = EMPTY;
				}
				file = 1;    // chess board, file
				i = 0;       // location in string
				while ((fenrank[i] != '\0') && (file < 9))
				{
					// '1'  through '8':
					if (((int)fenrank[i] > 48) && ((int)fenrank[i] < 57))
					{
						file += (int)fenrank[i] - 48;
					}
					else
						//  other characters:
					{
						switch (fenrank[i])
						{
						case 'P':
							board.square[BOARDINDEX[file][rank]] = WHITE_PAWN;
							file += 1;
							break;

						case 'N':
							board.square[BOARDINDEX[file][rank]] = WHITE_KNIGHT;
							file += 1;
							break;

						case 'B':
							board.square[BOARDINDEX[file][rank]] = WHITE_BISHOP;
							file += 1;
							break;

						case 'R':
							board.square[BOARDINDEX[file][rank]] = WHITE_ROOK;
							file += 1;
							break;

						case 'Q':
							board.square[BOARDINDEX[file][rank]] = WHITE_QUEEN;
							file += 1;
							break;

						case 'K':
							board.square[BOARDINDEX[file][rank]] = WHITE_KING;
							file += 1;
							break;

						case 'p':
							board.square[BOARDINDEX[file][rank]] = BLACK_PAWN;
							file += 1;
							break;

						case 'n':
							board.square[BOARDINDEX[file][rank]] = BLACK_KNIGHT;
							file += 1;
							break;

						case 'b':
							board.square[BOARDINDEX[file][rank]] = BLACK_BISHOP;
							file += 1;
							break;

						case 'r':
							board.square[BOARDINDEX[file][rank]] = BLACK_ROOK;
							file += 1;
							break;

						case 'q':
							board.square[BOARDINDEX[file][rank]] = BLACK_QUEEN;
							file += 1;
							break;

						case 'k':
							board.square[BOARDINDEX[file][rank]] = BLACK_KING;
							file += 1;
							break;

						default:
							break;
						}
					}
					i++;
				}
				board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
			}
		}

		else if (!strcmp(s, "r"))
		{
			board.viewRotated = !board.viewRotated;
		}

		else if (!strcmp(s, "white"))
		{
			next = BLACK_MOVE;
			board.initFromSquares(board.square, next, halfmoves, whiteCastle, blackCastle, epsq);
		}
		else
		{
			cout << "    command not implemented: " << s << ", type 'help' for more info" << endl;
			cin.clear();
		}

	}
}