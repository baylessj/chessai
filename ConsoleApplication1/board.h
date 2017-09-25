#pragma once

#include "bitops.h"
#include "movement.h"

class Board {
public:
	BitMap whiteKing, whiteQueens, whiteRooks, whiteBishops, whiteKnights, whitePawns;
	BitMap blackKing, blackQueens, blackRooks, blackBishops, blackKnights, blackPawns;
	BitMap whitePieces, blackPieces, occupiedSquares;

	unsigned char nextMove;        // WHITE_MOVE or BLACK_MOVE
	unsigned char castleWhite;     // White's castle status, CANCASTLEOO = 1, CANCASTLEOOO = 2
	unsigned char castleBlack;     // Black's castle status, CANCASTLEOO = 1, CANCASTLEOOO = 2
	int epSquare;                  // En-passant target square after double pawn move
	int fiftyMove;                 // Moves since the last pawn move or capture

	// additional variables:
	int Material;                  // incrementally updated, total material on board,
								   // in centipawns, from white's side of view
	int square[64];                // incrementally updated, this array is usefull if we want to
								   // probe what kind of piece is on a particular square.
	bool viewRotated;              // only used for displaying the board. TRUE or FALSE.

								   // storing moves:
	Move moveBuffer[MAX_MOV_BUFF]; // all generated moves of the current search tree are stored in this array.
	int moveBufLen[MAX_PLY];       // this arrays keeps track of which moves belong to which ply

	void init();
	void initFromSquares(int input[64], unsigned char next, int fiftyM, int castleW, int castleB, int epSq);
	void initFromFen(char fen[], char fencolor[], char fencastling[], char fenenpassant[], int fenhalfmoveclock, int fenfullmovenumber);
	void display();
};

extern Board board;

extern const int A8; extern const int B8; extern const int C8; extern const int D8;
extern const int E8; extern const int F8; extern const int G8; extern const int H8;
extern const int A7; extern const int B7; extern const int C7; extern const int D7;
extern const int E7; extern const int F7; extern const int G7; extern const int H7;
extern const int A6; extern const int B6; extern const int C6; extern const int D6;
extern const int E6; extern const int F6; extern const int G6; extern const int H6;
extern const int A5; extern const int B5; extern const int C5; extern const int D5;
extern const int E5; extern const int F5; extern const int G5; extern const int H5;
extern const int A4; extern const int B4; extern const int C4; extern const int D4;
extern const int E4; extern const int F4; extern const int G4; extern const int H4;
extern const int A3; extern const int B3; extern const int C3; extern const int D3;
extern const int E3; extern const int F3; extern const int G3; extern const int H3;
extern const int A2; extern const int B2; extern const int C2; extern const int D2;
extern const int E2; extern const int F2; extern const int G2; extern const int H2;
extern const int A1; extern const int B1; extern const int C1; extern const int D1;
extern const int E1; extern const int F1; extern const int G1; extern const int H1;

extern const char* SQUARENAME[];

extern const int FILES[];
extern const int RANKS[];