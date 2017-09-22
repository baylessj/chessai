#include <iostream>
#include <iomanip>
#include "cli.h"
#include "board.h"
#include "pieces.h"

Board board;

extern const int A8 = 56; extern const int B8 = 57; extern const int C8 = 58; extern const int D8 = 59;
extern const int E8 = 60; extern const int F8 = 61; extern const int G8 = 62; extern const int H8 = 63;
extern const int A7 = 48; extern const int B7 = 49; extern const int C7 = 50; extern const int D7 = 51;
extern const int E7 = 52; extern const int F7 = 53; extern const int G7 = 54; extern const int H7 = 55;
extern const int A6 = 40; extern const int B6 = 41; extern const int C6 = 42; extern const int D6 = 43;
extern const int E6 = 44; extern const int F6 = 45; extern const int G6 = 46; extern const int H6 = 47;
extern const int A5 = 32; extern const int B5 = 33; extern const int C5 = 34; extern const int D5 = 35;
extern const int E5 = 36; extern const int F5 = 37; extern const int G5 = 38; extern const int H5 = 39;
extern const int A4 = 24; extern const int B4 = 25; extern const int C4 = 26; extern const int D4 = 27;
extern const int E4 = 28; extern const int F4 = 29; extern const int G4 = 30; extern const int H4 = 31;
extern const int A3 = 16; extern const int B3 = 17; extern const int C3 = 18; extern const int D3 = 19;
extern const int E3 = 20; extern const int F3 = 21; extern const int G3 = 22; extern const int H3 = 23;
extern const int A2 = 8; extern const int B2 = 9; extern const int C2 = 10; extern const int D2 = 11;
extern const int E2 = 12; extern const int F2 = 13; extern const int G2 = 14; extern const int H2 = 15;
extern const int A1 = 0; extern const int B1 = 1; extern const int C1 = 2; extern const int D1 = 3;
extern const int E1 = 4; extern const int F1 = 5; extern const int G1 = 6; extern const int H1 = 7;

const char* SQUARENAME[64] = { "a1","b1","c1","d1","e1","f1","g1","h1",
"a2","b2","c2","d2","e2","f2","g2","h2",
"a3","b3","c3","d3","e3","f3","g3","h3",
"a4","b4","c4","d4","e4","f4","g4","h4",
"a5","b5","c5","d5","e5","f5","g5","h5",
"a6","b6","c6","d6","e6","f6","g6","h6",
"a7","b7","c7","d7","e7","f7","g7","h7",
"a8","b8","c8","d8","e8","f8","g8","h8" };

extern const int FILES[64] = {
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8
};

extern const int RANKS[64] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8
};

void Board::init()
{
	viewRotated = false;

	for (int i = 0; i < 64; i++) square[i] = EMPTY;

	square[E1] = WHITE_KING;
	square[D1] = WHITE_QUEEN;
	square[A1] = WHITE_ROOK;
	square[H1] = WHITE_ROOK;
	square[B1] = WHITE_KNIGHT;
	square[G1] = WHITE_KNIGHT;
	square[C1] = WHITE_BISHOP;
	square[F1] = WHITE_BISHOP;
	square[A2] = WHITE_PAWN;
	square[B2] = WHITE_PAWN;
	square[C2] = WHITE_PAWN;
	square[D2] = WHITE_PAWN;
	square[E2] = WHITE_PAWN;
	square[F2] = WHITE_PAWN;
	square[G2] = WHITE_PAWN;
	square[H2] = WHITE_PAWN;

	square[E8] = BLACK_KING;
	square[D8] = BLACK_QUEEN;
	square[A8] = BLACK_ROOK;
	square[H8] = BLACK_ROOK;
	square[B8] = BLACK_KNIGHT;
	square[G8] = BLACK_KNIGHT;
	square[C8] = BLACK_BISHOP;
	square[F8] = BLACK_BISHOP;
	square[A7] = BLACK_PAWN;
	square[B7] = BLACK_PAWN;
	square[C7] = BLACK_PAWN;
	square[D7] = BLACK_PAWN;
	square[E7] = BLACK_PAWN;
	square[F7] = BLACK_PAWN;
	square[G7] = BLACK_PAWN;
	square[H7] = BLACK_PAWN;

	initFromSquares(square, WHITE_MOVE, 0, CANCASTLEOO + CANCASTLEOOO, CANCASTLEOO + CANCASTLEOOO, 0);

	return;
}

void Board::initFromSquares(int input[64], unsigned char next, int fiftyM, int castleW, int castleB, int epSq)
{
	// sets up the board variables according to the information found in
	// the input[64] array
	// All board & game initializations are done through this function (including readfen and setup).

	// bitboards
	whiteKing = 0;
	whiteQueens = 0;
	whiteRooks = 0;
	whiteBishops = 0;
	whiteKnights = 0;
	whitePawns = 0;
	blackKing = 0;
	blackQueens = 0;
	blackRooks = 0;
	blackBishops = 0;
	blackKnights = 0;
	blackPawns = 0;
	whitePieces = 0;
	blackPieces = 0;
	occupiedSquares = 0;

	// populate the 12 bitboard:
	for (int i = 0; i < 64; i++)
	{
		square[i] = input[i];
		if (square[i] == WHITE_KING)   whiteKing = whiteKing | BITSET[i];
		if (square[i] == WHITE_QUEEN)  whiteQueens = whiteQueens | BITSET[i];
		if (square[i] == WHITE_ROOK)   whiteRooks = whiteRooks | BITSET[i];
		if (square[i] == WHITE_BISHOP) whiteBishops = whiteBishops | BITSET[i];
		if (square[i] == WHITE_KNIGHT) whiteKnights = whiteKnights | BITSET[i];
		if (square[i] == WHITE_PAWN)   whitePawns = whitePawns | BITSET[i];
		if (square[i] == BLACK_KING)   blackKing = blackKing | BITSET[i];
		if (square[i] == BLACK_QUEEN)  blackQueens = blackQueens | BITSET[i];
		if (square[i] == BLACK_ROOK)   blackRooks = blackRooks | BITSET[i];
		if (square[i] == BLACK_BISHOP) blackBishops = blackBishops | BITSET[i];
		if (square[i] == BLACK_KNIGHT) blackKnights = blackKnights | BITSET[i];
		if (square[i] == BLACK_PAWN)   blackPawns = blackPawns | BITSET[i];
	}

	whitePieces = whiteKing | whiteQueens | whiteRooks | whiteBishops | whiteKnights | whitePawns;
	blackPieces = blackKing | blackQueens | blackRooks | blackBishops | blackKnights | blackPawns;
	occupiedSquares = whitePieces | blackPieces;

	nextMove = next;
	castleWhite = castleW;
	castleBlack = castleB;
	epSquare = epSq;
	fiftyMove = fiftyM;

	Material = bitCnt(whitePawns) * PAWN_VALUE +
		bitCnt(whiteKnights) * KNIGHT_VALUE +
		bitCnt(whiteBishops) * BISHOP_VALUE +
		bitCnt(whiteRooks) * ROOK_VALUE +
		bitCnt(whiteQueens) * QUEEN_VALUE;
	Material -= (bitCnt(blackPawns) * PAWN_VALUE +
		bitCnt(blackKnights) * KNIGHT_VALUE +
		bitCnt(blackBishops) * BISHOP_VALUE +
		bitCnt(blackRooks) * ROOK_VALUE +
		bitCnt(blackQueens) * QUEEN_VALUE);

	return;
}

void Board::display()
{
	int rank, file;

	std::cout << std::endl;
	{
		if (!viewRotated)
		{
			for (rank = 8; rank >= 1; rank--)
			{
				std::cout << "    +---+---+---+---+---+---+---+---+" << std::endl;
				std::cout << std::setw(3) << rank << " |";
				for (file = 1; file <= 8; file++)
				{
					std::cout << " " << PIECENAMES[square[BOARDINDEX[file][rank]]] << "|";
				}
				std::cout << std::endl;
			}
			std::cout << "    +---+---+---+---+---+---+---+---+" << std::endl;
			std::cout << "      a   b   c   d   e   f   g   h" << std::endl << std::endl;
		}
		else
		{
			std::cout << "      h   g   f   e   d   c   b   a" << std::endl;
			for (rank = 1; rank <= 8; rank++)
			{
				std::cout << "    +---+---+---+---+---+---+---+---+" << std::endl;
				std::cout << "    |";
				for (file = 8; file >= 1; file--)
				{
					std::cout << " " << PIECENAMES[square[BOARDINDEX[file][rank]]] << "|";
				}
				std::cout << std::setw(3) << rank << std::endl;
			}
			std::cout << "    +---+---+---+---+---+---+---+---+" << std::endl << std::endl;
		}
	}
	return;
}