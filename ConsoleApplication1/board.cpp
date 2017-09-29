#include <iostream>
#include <iomanip>
#include "cli.h"
#include "board.h"
#include "pieces.h"
#include "eval.h"

Board board;

const int A8 = 56;  const int B8 = 57;  const int C8 = 58;  const int D8 = 59;
 const int E8 = 60;  const int F8 = 61;  const int G8 = 62;  const int H8 = 63;
 const int A7 = 48;  const int B7 = 49;  const int C7 = 50;  const int D7 = 51;
 const int E7 = 52;  const int F7 = 53;  const int G7 = 54;  const int H7 = 55;
 const int A6 = 40;  const int B6 = 41;  const int C6 = 42;  const int D6 = 43;
 const int E6 = 44;  const int F6 = 45;  const int G6 = 46;  const int H6 = 47;
 const int A5 = 32;  const int B5 = 33;  const int C5 = 34;  const int D5 = 35;
 const int E5 = 36;  const int F5 = 37;  const int G5 = 38;  const int H5 = 39;
 const int A4 = 24;  const int B4 = 25;  const int C4 = 26;  const int D4 = 27;
 const int E4 = 28;  const int F4 = 29;  const int G4 = 30;  const int H4 = 31;
 const int A3 = 16;  const int B3 = 17;  const int C3 = 18;  const int D3 = 19;
 const int E3 = 20;  const int F3 = 21;  const int G3 = 22;  const int H3 = 23;
 const int A2 = 8;  const int B2 = 9;  const int C2 = 10;  const int D2 = 11;
 const int E2 = 12;  const int F2 = 13;  const int G2 = 14;  const int H2 = 15;
 const int A1 = 0;  const int B1 = 1;  const int C1 = 2;  const int D1 = 3;
 const int E1 = 4;  const int F1 = 5;  const int G1 = 6;  const int H1 = 7;

const char* SQUARENAME[64] = { "a1","b1","c1","d1","e1","f1","g1","h1",
"a2","b2","c2","d2","e2","f2","g2","h2",
"a3","b3","c3","d3","e3","f3","g3","h3",
"a4","b4","c4","d4","e4","f4","g4","h4",
"a5","b5","c5","d5","e5","f5","g5","h5",
"a6","b6","c6","d6","e6","f6","g6","h6",
"a7","b7","c7","d7","e7","f7","g7","h7",
"a8","b8","c8","d8","e8","f8","g8","h8" };

 const int FILES[64] = {
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8,
	1, 2, 3, 4, 5, 6, 7, 8
};

 const int RANKS[64] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8
};


// Evaluation scores start here, all scores are in centipawns.
// If there are scoring data that is supposed to be symmetrical (i.e. same for BLACK & WHITE),
// then only the data for WHITE is supplied, and data for BLACK is calculated in dataInit().
// This is done to make data entry easier, eliminate typos and guarantuee symmetry.
//
// MIRRORED:
// Some scoring arrays are supplied MIRRORED, i.e. starting with the last rank (see the comments below).
// They are mirrored back in the right order in dataInit().
// This is only done to make data entry easier, because you can enter the scoring data as if you're
// looking at the chess board from White's point of perspective.

int PENALTY_DOUBLED_PAWN = 10;
int PENALTY_ISOLATED_PAWN = 20;
int PENALTY_BACKWARD_PAWN = 8;
int BONUS_PASSED_PAWN = 20;
int BONUS_BISHOP_PAIR = 10;
int BONUS_ROOK_BEHIND_PASSED_PAWN = 20;
int BONUS_ROOK_ON_OPEN_FILE = 20;
int BONUS_TWO_ROOKS_ON_OPEN_FILE = 20;

int BONUS_PAWN_SHIELD_STRONG = 9;
int BONUS_PAWN_SHIELD_WEAK = 4;

int PAWN_OWN_DISTANCE[8] = { 0,   8,  4,  2,  0,  0,  0,  0 };
int PAWN_OPPONENT_DISTANCE[8] = { 0,   2,  1,  0,  0,  0,  0,  0 };
int KNIGHT_DISTANCE[8] = { 0,   4,  4,  0,  0,  0,  0,  0 };
int BISHOP_DISTANCE[8] = { 0,   5,  4,  3,  2,  1,  0,  0 };
int ROOK_DISTANCE[8] = { 0,   7,  5,  4,  3,  0,  0,  0 };
int QUEEN_DISTANCE[8] = { 0,  10,  8,  5,  4,  0,  0,  0 };

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int PAWNPOS_W[64] = {
	0,   0,   0,   0,   0,   0,   0,   0,
	5,  10,  15,  20,  20,  15,  10,   5,
	4,   8,  12,  16,  16,  12,   8,   4,
	3,   6,   9,  12,  12,   9,   6,   3,
	2,   4,   6,   8,   8,   6,   4,   2,
	1,   2,   3, -10, -10,   3,   2,   1,
	0,   0,   0, -40, -40,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0
};

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int KNIGHTPOS_W[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -30, -10
};

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int BISHOPPOS_W[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int ROOKPOS_W[64] = {
	0,  0,  0,  0,   0,  0,  0,   0,
	15, 15, 15, 15,  15, 15, 15,  15,
	0,  0,  0,  0,   0,  0,  0,   0,
	0,  0,  0,  0,   0,  0,  0,   0,
	0,  0,  0,  0,   0,  0,  0,   0,
	0,  0,  0,  0,   0,  0,  0,   0,
	0,  0,  0,  0,   0,  0,  0,   0,
	-10,  0,  0, 10,  10,  0,  0, -10
};

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int QUEENPOS_W[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int KINGPOS_W[64] = {
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-20, -20, -20, -20, -20, -20, -20, -20,
	0,  20,  40, -20,   0, -20,  40,  20
};

// *** This array is MIRRORED                          ***
// *** You can enter the scoring data as if you're     ***
// *** looking at the chess board from white's point   ***
// *** of perspective. Lower left corner is square a1: ***
int KINGPOS_ENDGAME_W[64] = {
	0,  10,  20,  30,  30,  20,  10,   0,
	10,  20,  30,  40,  40,  30,  20,  10,
	20,  30,  40,  50,  50,  40,  30,  20,
	30,  40,  50,  60,  60,  50,  40,  30,
	30,  40,  50,  60,  60,  50,  40,  30,
	20,  30,  40,  50,  50,  40,  30,  20,
	10,  20,  30,  40,  40,  30,  20,  10,
	0,  10,  20,  30,  30,  20,  10,   0
};

int MIRROR[64] = {
	56,  57,  58,  59,  60,  61,  62,  63,
	48,  49,  50,  51,  52,  53,  54,  55,
	40,  41,  42,  43,  44,  45,  46,  47,
	32,  33,  34,  35,  36,  37,  38,  39,
	24,  25,  26,  27,  28,  29,  30,  31,
	16,  17,  18,  19,  20,  21,  22,  23,
	8,   9,  10,  11,  12,  13,  14,  15,
	0,   1,   2,   3,   4,   5,   6,   7
};

int DISTANCE[64][64];
int PAWNPOS_B[64];
int KNIGHTPOS_B[64];
int BISHOPPOS_B[64];
int ROOKPOS_B[64];
int QUEENPOS_B[64];
int KINGPOS_B[64];
int KINGPOS_ENDGAME_B[64];
BitMap PASSED_WHITE[64];
BitMap PASSED_BLACK[64];
BitMap ISOLATED_WHITE[64];
BitMap ISOLATED_BLACK[64];
BitMap BACKWARD_WHITE[64];
BitMap BACKWARD_BLACK[64];
BitMap KINGSHIELD_STRONG_W[64];
BitMap KINGSHIELD_STRONG_B[64];
BitMap KINGSHIELD_WEAK_W[64];
BitMap KINGSHIELD_WEAK_B[64];
BitMap WHITE_SQUARES;
BitMap BLACK_SQUARES;

static void evalInit() {
	int i, rank, square;
	//     ===========================================================================
	//     Initialize evaluation data & bitmaps
	//     ===========================================================================

	BLACK_SQUARES = 0;
	for (i = 0; i < 64; i++)
	{
		if ((i + RANKS[i]) % 2) BLACK_SQUARES ^= BITSET[i];
	}
	WHITE_SQUARES = ~BLACK_SQUARES;

	// Clear bitmaps:
	for (i = 0; i < 64; i++)
	{
		PASSED_WHITE[i] = 0;
		PASSED_BLACK[i] = 0;
		ISOLATED_WHITE[i] = 0;
		ISOLATED_BLACK[i] = 0;
		BACKWARD_WHITE[i] = 0;
		BACKWARD_BLACK[i] = 0;
		KINGSHIELD_STRONG_W[i] = 0;
		KINGSHIELD_STRONG_B[i] = 0;
		KINGSHIELD_WEAK_W[i] = 0;
		KINGSHIELD_WEAK_B[i] = 0;
	}

	for (i = 0; i < 64; i++)
	{
		//  PASSED_WHITE:
		for (rank = RANKS[i] + 1; rank < 8; rank++)
		{
			// 3 files:
			if (FILES[i] - 1 > 0) PASSED_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i] - 1][rank]];
			PASSED_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i]][rank]];
			if (FILES[i] + 1 < 9) PASSED_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i] + 1][rank]];
		}

		// ISOLATED_WHITE:
		for (rank = 2; rank < 8; rank++)
		{
			// 2 files:
			if (FILES[i] - 1 > 0) ISOLATED_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i] - 1][rank]];
			if (FILES[i] + 1 < 9) ISOLATED_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i] + 1][rank]];
		}

		//  BACKWARD_WHITE:
		for (rank = 2; rank <= RANKS[i]; rank++)
		{
			// 2 files:
			if (FILES[i] - 1 > 0) BACKWARD_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i] - 1][rank]];
			if (FILES[i] + 1 < 9) BACKWARD_WHITE[i] ^= BITSET[BOARDINDEX[FILES[i] + 1][rank]];
		}
	}

	// Pawn shield bitmaps for king safety, only if the king is on the first 3 ranks:
	for (i = 0; i < 24; i++)
	{
		//  KINGSHIELD_STRONG_W & KINGSHIELD_WEAK_W:
		KINGSHIELD_STRONG_W[i] ^= BITSET[i + 8];
		KINGSHIELD_WEAK_W[i] ^= BITSET[i + 16];
		if (FILES[i] > 1)
		{
			KINGSHIELD_STRONG_W[i] ^= BITSET[i + 7];
			KINGSHIELD_WEAK_W[i] ^= BITSET[i + 15];
		}
		if (FILES[i] < 8)
		{
			KINGSHIELD_STRONG_W[i] ^= BITSET[i + 9];
			KINGSHIELD_WEAK_W[i] ^= BITSET[i + 17];
		}
		if (FILES[i] == 1)
		{
			KINGSHIELD_STRONG_W[i] ^= BITSET[i + 10];
			KINGSHIELD_WEAK_W[i] ^= BITSET[i + 18];
		}
		if (FILES[i] == 8)
		{
			KINGSHIELD_STRONG_W[i] ^= BITSET[i + 6];
			KINGSHIELD_WEAK_W[i] ^= BITSET[i + 14];
		}
	}

	//     ===========================================================================
	//     DISTANCE array, distance is measured as max of (rank,file)-difference
	//     ===========================================================================
	for (i = 0; i < 64; i++)
	{
		for (square = 0; square < 64; square++)
		{
			if (abs(RANKS[i] - RANKS[square]) > abs(FILES[i] - FILES[square]))
				DISTANCE[i][square] = abs(RANKS[i] - RANKS[square]);
			else
				DISTANCE[i][square] = abs(FILES[i] - FILES[square]);
		}
	}

	//     ===========================================================================
	//     Initialize MIRRORed data:
	//     ===========================================================================
	// Data is supplied as mirrored for WHITE, so it's ready for BLACK to use:
	for (square = 0; square < 64; square++)
	{
		PAWNPOS_B[square] = PAWNPOS_W[square];
		KNIGHTPOS_B[square] = KNIGHTPOS_W[square];
		BISHOPPOS_B[square] = BISHOPPOS_W[square];
		ROOKPOS_B[square] = ROOKPOS_W[square];
		QUEENPOS_B[square] = QUEENPOS_W[square];
		KINGPOS_B[square] = KINGPOS_W[square];
		KINGPOS_ENDGAME_B[square] = KINGPOS_ENDGAME_W[square];
	}

	// Complete missing mirrored data:
	for (i = 0; i < 64; i++)
	{
		PAWNPOS_W[i] = PAWNPOS_B[MIRROR[i]];
		KNIGHTPOS_W[i] = KNIGHTPOS_B[MIRROR[i]];
		BISHOPPOS_W[i] = BISHOPPOS_B[MIRROR[i]];
		ROOKPOS_W[i] = ROOKPOS_B[MIRROR[i]];
		QUEENPOS_W[i] = QUEENPOS_B[MIRROR[i]];
		KINGPOS_W[i] = KINGPOS_B[MIRROR[i]];
		KINGPOS_ENDGAME_W[i] = KINGPOS_ENDGAME_B[MIRROR[i]];

		for (square = 0; square < 64; square++)
		{
			//  PASSED_BLACK bitmaps (mirror of PASSED_WHITE bitmaps):
			if (PASSED_WHITE[i] & BITSET[square]) PASSED_BLACK[MIRROR[i]] |= BITSET[MIRROR[square]];

			//  ISOLATED_BLACK bitmaps (mirror of ISOLATED_WHITE bitmaps):
			if (ISOLATED_WHITE[i] & BITSET[square]) ISOLATED_BLACK[MIRROR[i]] |= BITSET[MIRROR[square]];

			//  BACKWARD_BLACK bitmaps (mirror of BACKWARD_WHITE bitmaps):
			if (BACKWARD_WHITE[i] & BITSET[square]) BACKWARD_BLACK[MIRROR[i]] |= BITSET[MIRROR[square]];

			//  KINGSHIELD_STRONG_B bitmaps (mirror of KINGSHIELD_STRONG_W bitmaps):
			if (KINGSHIELD_STRONG_W[i] & BITSET[square]) KINGSHIELD_STRONG_B[MIRROR[i]] |= BITSET[MIRROR[square]];

			//  KINGSHIELD_WEAK_B bitmaps (mirror of KINGSHIELD_WEAK_W bitmaps):
			if (KINGSHIELD_WEAK_W[i] & BITSET[square]) KINGSHIELD_WEAK_B[MIRROR[i]] |= BITSET[MIRROR[square]];
		}
	}
}

/**
* @brief Initializes the board squares and piece starting spots
*/
void Board::init() {
	viewRotated = false;

	// BOARDINDEX is used to translate [file][rank] to [square],
	// Note that file is from 1..8 and rank from 1..8 (not starting from 0)
	for (int rank = 0; rank < 9; rank++) {
		for (int file = 0; file < 9; file++) {
			BOARDINDEX[file][rank] = (rank - 1) * 8 + file - 1;
		}
	}

	evalInit();

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

/**
* @brief Allows for custom initialization of the board. 
*        Both Setup and FEN initialization are done with this routine 
*
* @param input
*        The information about each square's initialization
* @param next
*        Which color has the next move
* @param movesPawn
*        The number of moves since a pawn has moved
* @param castleW
*        White's castle status
* @param castleB
*        Black's castle status
* @param epSq
*        En passant target square after double pawn move
*/
void Board::initFromSquares(int input[64], unsigned char next, int movesPawn, int castleW, int castleB, int epSq) {
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
	for (int i = 0; i < 64; i++) {
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
	movesSincePawn = movesPawn;

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

/**
* @brief displays the board in the console
*/
void Board::display()
{
	int rank, file;

	using namespace std;

	cout << endl;
	{
		if (!viewRotated)
		{
			for (rank = 8; rank >= 1; rank--)
			{
				cout << "    +---+---+---+---+---+---+---+---+" << endl;
				cout << setw(3) << rank << " |";
				for (file = 1; file <= 8; file++)
				{
					cout << " " << PIECENAMES[square[BOARDINDEX[file][rank]]] << "|";
				}
				cout << endl;
			}
			cout << "    +---+---+---+---+---+---+---+---+" << endl;
			cout << "      a   b   c   d   e   f   g   h" << endl << endl;
		}
		else
		{
			cout << "      h   g   f   e   d   c   b   a" << endl;
			for (rank = 1; rank <= 8; rank++)
			{
				cout << "    +---+---+---+---+---+---+---+---+" << endl;
				cout << "    |";
				for (file = 8; file >= 1; file--)
				{
					cout << " " << PIECENAMES[square[BOARDINDEX[file][rank]]] << "|";
				}
				cout << setw(3) << rank << endl;
			}
			cout << "    +---+---+---+---+---+---+---+---+" << endl << endl;
		}
	}
	return;
}

int Board::eval()
{

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// This is winglet's evaluation function
	// The score is calculated from White's perspective (in centipawns)
	// and returned from the perspective of the side to move.
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	int score, square;
	int whitepawns, whiteknights, whitebishops, whiterooks, whitequeens;
	int blackpawns, blackknights, blackbishops, blackrooks, blackqueens;
	int whitekingsquare, blackkingsquare;
	int whitetotalmat, blacktotalmat;
	int whitetotal, blacktotal;
	bool endgame;
	BitMap temp, whitepassedpawns, blackpassedpawns;

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Material
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	score = board.Material;
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> MATERIAL>                       " << board.Material << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Remember where the kings are
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	whitekingsquare = firstOne(board.whiteKing);
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> WHITE KING POSITION>            " << SQUARENAME[whitekingsquare] << std::endl;
#endif
	blackkingsquare = firstOne(board.blackKing);
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> BLACK KING POSITION>            " << SQUARENAME[blackkingsquare] << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Piece counts, note that we could have done this incrementally in (un)makeMove
	// because it's basically the same thing as keeping board.Material up to date..
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	whitepawns = bitCnt(board.whitePawns);
	whiteknights = bitCnt(board.whiteKnights);
	whitebishops = bitCnt(board.whiteBishops);
	whiterooks = bitCnt(board.whiteRooks);
	whitequeens = bitCnt(board.whiteQueens);
	whitetotalmat = 3 * whiteknights + 3 * whitebishops + 5 * whiterooks + 10 * whitequeens;
	whitetotal = whitepawns + whiteknights + whitebishops + whiterooks + whitequeens;
	blackpawns = bitCnt(board.blackPawns);
	blackknights = bitCnt(board.blackKnights);
	blackbishops = bitCnt(board.blackBishops);
	blackrooks = bitCnt(board.blackRooks);
	blackqueens = bitCnt(board.blackQueens);
	blacktotalmat = 3 * blackknights + 3 * blackbishops + 5 * blackrooks + 10 * blackqueens;
	blacktotal = blackpawns + blackknights + blackbishops + blackrooks + blackqueens;
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> WHITE TOTAL MAT>                " << whitetotalmat << std::endl;
	std::cout << "EVAL> BLACK TOTAL MAT>                " << blacktotalmat << std::endl;
	std::cout << "EVAL> WHITE TOTAL>                    " << whitetotal << std::endl;
	std::cout << "EVAL> BLACK TOTAL>                    " << blacktotal << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Check if we are in the endgame
	// Anything less than a queen (=10) + rook (=5) is considered endgame
	// (pawns excluded in this count)
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	endgame = (whitetotalmat < 15 || blacktotalmat < 15);
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> ENDGAME>                        " << endgame << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate for draws due to insufficient material:
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (!whitepawns && !blackpawns)
	{
		// king versus king:
		if ((whitetotalmat == 0) && (blacktotalmat == 0)) return 0;

		// king and knight versus king:
		if (((whitetotalmat == 3) && (whiteknights == 1) && (blacktotalmat == 0)) ||
			((blacktotalmat == 3) && (blackknights == 1) && (whitetotalmat == 0))) return 0;

		// 2 kings with one or more bishops, and all bishops on the same colour:
		if ((whitebishops + blackbishops) > 0)
		{
			if ((whiteknights == 0) && (whiterooks == 0) && (whitequeens == 0) &&
				(blackknights == 0) && (blackrooks == 0) && (blackqueens == 0))
			{
				if (!((board.whiteBishops | board.blackBishops) & WHITE_SQUARES) ||
					!((board.whiteBishops | board.blackBishops) & BLACK_SQUARES)) return 0;
			}
		}
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate MATERIAL
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Have the winning side prefer to exchange pieces
	// Every exchange with unequal material adds 3 centipawns to the score
	// Loosing a piece (from balanced material) becomes more
	// severe in the endgame
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WINGLET_VERBOSE_EVAL
	int iexch = 0;
	int iwhitepos = 0;
	int iwhitepawns = 0;
	int iwhiteking = 0;
	int iblackpos = 0;
	int iblackpawns = 0;
	int iblackking = 0;
#endif

	if (whitetotalmat + whitepawns > blacktotalmat + blackpawns)
	{
		score += 45 + 3 * whitetotal - 6 * blacktotal;
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> EXCHANGE WHITE>                 " << (45 + 3 * whitetotal - 6 * blacktotal) << std::endl;
		iexch = (45 + 3 * whitetotal - 6 * blacktotal);
#endif
	}
	else
	{
		if (whitetotalmat + whitepawns < blacktotalmat + blackpawns)
		{
			score += -45 - 3 * blacktotal + 6 * whitetotal;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL> EXCHANGE BLACK>                 " << (-45 - 3 * blacktotal + 6 * whitetotal) << std::endl;
			iexch = (-45 - 3 * blacktotal + 6 * whitetotal);
#endif
		}
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate WHITE PIECES
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white pawns
	// - position on the board
	// - distance from opponent king
	// - distance from own king
	// - passed, doubled, isolated or backward pawns
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	whitepassedpawns = 0;
	temp = board.whitePawns;
	while (temp)
	{
		square = firstOne(temp);

#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE PAWN ON                   " << SQUARENAME[square] << std::endl;
#endif

		score += PAWNPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << PAWNPOS_W[square] << std::endl;
		iwhitepos += PAWNPOS_W[square];
		iwhitepawns += PAWNPOS_W[square];
#endif

		score += PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]];
		iwhitepawns += PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		if (endgame)
		{
			score += PAWN_OWN_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  DISTANCE TO OWN KING SCORE     " << PAWN_OWN_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
			iwhitepos += PAWN_OWN_DISTANCE[DISTANCE[square][whitekingsquare]];
			iwhitepawns += PAWN_OWN_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif
		}

		if (!(PASSED_WHITE[square] & board.blackPawns))
		{
			score += BONUS_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS PASSED, BONUS IS            " << BONUS_PASSED_PAWN << std::endl;
			iwhitepos += BONUS_PASSED_PAWN;
			iwhitepawns += BONUS_PASSED_PAWN;
#endif

			// remember its location, we need it later when evaluating the white rooks:
			whitepassedpawns ^= BITSET[square];
		}

		if ((board.whitePawns ^ BITSET[square]) & FILEMASK[square])
		{
			score -= PENALTY_DOUBLED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS DOUBLED, PENALTY IS         " << PENALTY_DOUBLED_PAWN << std::endl;
			iwhitepos -= PENALTY_DOUBLED_PAWN;
			iwhitepawns -= PENALTY_DOUBLED_PAWN;
#endif
		}

		if (!(ISOLATED_WHITE[square] & board.whitePawns))
		{
			score -= PENALTY_ISOLATED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS ISOLATED, PENALTY IS        " << PENALTY_ISOLATED_PAWN << std::endl;
			iwhitepos -= PENALTY_ISOLATED_PAWN;
			iwhitepawns -= PENALTY_ISOLATED_PAWN;
#endif
		}
		else
		{
			// If it is not isolated, then it might be backward. Two conditions must be true:
			//  1) if the next square is controlled by an enemy pawn - we use the PAWN_ATTACKS bitmaps to check this
			//  2) if there are no pawns left that could defend this pawn
			if ((WHITE_PAWN_ATTACKS[square + 8] & board.blackPawns))
			{
				if (!(BACKWARD_WHITE[square] & board.whitePawns))
				{
					score -= PENALTY_BACKWARD_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
					std::cout << "EVAL>  IS BACKWARD, PENALTY IS        " << PENALTY_BACKWARD_PAWN << std::endl;
					iwhitepos -= PENALTY_BACKWARD_PAWN;
					iwhitepawns -= PENALTY_BACKWARD_PAWN;
#endif
				}
			}
		}

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white knights
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.whiteKnights;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE KNIGHT ON                 " << SQUARENAME[square] << std::endl;
#endif

		score += KNIGHTPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KNIGHTPOS_W[square] << std::endl;
		iwhitepos += KNIGHTPOS_W[square];
#endif

		score += KNIGHT_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << KNIGHT_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += KNIGHT_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white bishops
	// - having the pair
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (board.whiteBishops)
	{
		if (board.whiteBishops & (board.whiteBishops - 1))
		{
			score += BONUS_BISHOP_PAIR;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL> WHITE BISHOP PAIR BONUS         " << BONUS_BISHOP_PAIR << std::endl;
			iwhitepos += BONUS_BISHOP_PAIR;
#endif              
		}
	}

	temp = board.whiteBishops;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE BISHOP ON                 " << SQUARENAME[square] << std::endl;
#endif

		score += BISHOPPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << BISHOPPOS_W[square] << std::endl;
		iwhitepos += BISHOPPOS_W[square];
#endif

		score += BISHOP_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << BISHOP_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += BISHOP_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white rooks
	// - position on the board
	// - distance from opponent king
	// - on the same file as a passed pawn
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.whiteRooks;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE ROOK ON                   " << SQUARENAME[square] << std::endl;
#endif

		score += ROOKPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << ROOKPOS_W[square] << std::endl;
		iwhitepos += ROOKPOS_W[square];
#endif

		score += ROOK_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << ROOK_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += ROOK_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		if (FILEMASK[square] & whitepassedpawns)
		{
			if ((unsigned int)square < lastOne(FILEMASK[square] & whitepassedpawns))
			{
				score += BONUS_ROOK_BEHIND_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
				std::cout << "EVAL>  BEHIND PASSED PAWN BONUS       " << BONUS_ROOK_BEHIND_PASSED_PAWN << std::endl;
				iwhitepos += BONUS_ROOK_BEHIND_PASSED_PAWN;
#endif
			}
		}
		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white queens
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.whiteQueens;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE QUEEN ON                  " << SQUARENAME[square] << std::endl;
#endif

		score += QUEENPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << QUEENPOS_W[square] << std::endl;
		iwhitepos += QUEENPOS_W[square];
#endif

		score += QUEEN_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << QUEEN_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += QUEEN_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate the white king
	// - position on the board
	// - proximity to the pawns
	// - pawn shield (not in the endgame)
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (endgame)
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE KING ON (endgame)         " << SQUARENAME[whitekingsquare] << std::endl;
#endif

		score += KINGPOS_ENDGAME_W[whitekingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_ENDGAME_W[whitekingsquare] << std::endl;
		iwhitepos += KINGPOS_ENDGAME_W[whitekingsquare];
		iwhiteking += KINGPOS_ENDGAME_W[whitekingsquare];
#endif

	}
	else
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE KING ON (no endgame)      " << SQUARENAME[whitekingsquare] << std::endl;
#endif

		score += KINGPOS_W[whitekingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_W[whitekingsquare] << std::endl;
		iwhitepos += KINGPOS_W[whitekingsquare];
		iwhiteking += KINGPOS_W[whitekingsquare];
#endif

		// add pawn shield bonus if we're not in the endgame:
		// strong pawn shield bonus if the pawns are near the king:
		score += BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  STRONG PAWN SHIELD SCORE IS    " << BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns) << std::endl;
		iwhitepos += BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns);
		iwhiteking += BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns);
#endif

		// weaker pawn shield bonus if the pawns are not so near the king:
		score += BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  WEAK PAWN SHIELD SCORE IS      " << BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns) << std::endl;
		iwhitepos += BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns);
		iwhiteking += BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns);
#endif

	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate BLACK PIECES
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black pawns
	// - position on the board
	// - distance from opponent king
	// - distance from own king
	// - passed, doubled, isolated or backward pawns
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	blackpassedpawns = 0;
	temp = board.blackPawns;
	while (temp)
	{
		square = firstOne(temp);

#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK PAWN ON                   " << SQUARENAME[square] << std::endl;
#endif

		score -= PAWNPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << PAWNPOS_B[square] << std::endl;
		iblackpos -= PAWNPOS_B[square];
		iblackpawns -= PAWNPOS_B[square];
#endif

		score -= PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]];
		iblackpawns -= PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		if (endgame)
		{
			score -= PAWN_OWN_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  DISTANCE TO OWN KING SCORE     " << PAWN_OWN_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
			iblackpos -= PAWN_OWN_DISTANCE[DISTANCE[square][blackkingsquare]];
			iblackpawns -= PAWN_OWN_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif
		}

		if (!(PASSED_BLACK[square] & board.whitePawns))
		{
			score -= BONUS_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS PASSED, BONUS IS            " << BONUS_PASSED_PAWN << std::endl;
			iblackpos -= BONUS_PASSED_PAWN;
			iblackpawns -= BONUS_PASSED_PAWN;
#endif

			// remember its location, we need it later when evaluating the black rooks:
			blackpassedpawns ^= BITSET[square];

		}

		if ((board.blackPawns ^ BITSET[square]) & FILEMASK[square])
		{
			score += PENALTY_DOUBLED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS DOUBLED, PENALTY IS         " << PENALTY_DOUBLED_PAWN << std::endl;
			iblackpos += PENALTY_DOUBLED_PAWN;
			iblackpawns += PENALTY_DOUBLED_PAWN;
#endif
		}

		if (!(ISOLATED_BLACK[square] & board.blackPawns))
		{
			score += PENALTY_ISOLATED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS ISOLATED, PENALTY IS        " << PENALTY_ISOLATED_PAWN << std::endl;
			iblackpos += PENALTY_ISOLATED_PAWN;
			iblackpawns += PENALTY_ISOLATED_PAWN;
#endif
		}
		else
		{
			// If it is not isolated, then it might be backward. Two conditions must be true:
			//  1) if the next square is controlled by an enemy pawn - we use the PAWN_ATTACKS bitmaps to check this
			//  2) if there are no pawns left that could defend this pawn
			if ((BLACK_PAWN_ATTACKS[square - 8] & board.whitePawns))
			{
				if (!(BACKWARD_BLACK[square] & board.blackPawns))
				{
					score += PENALTY_BACKWARD_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
					std::cout << "EVAL>  IS BACKWARD, PENALTY IS        " << PENALTY_BACKWARD_PAWN << std::endl;
					iwhitepos += PENALTY_BACKWARD_PAWN;
					iwhitepawns += PENALTY_BACKWARD_PAWN;
#endif
				}
			}
		}

		temp ^= BITSET[square];

	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black knights
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.blackKnights;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK KNIGHT ON                 " << SQUARENAME[square] << std::endl;
#endif

		score -= KNIGHTPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KNIGHTPOS_B[square] << std::endl;
		iblackpos -= KNIGHTPOS_B[square];
#endif

		score -= KNIGHT_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << KNIGHT_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= KNIGHT_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black bishops
	// - having the pair
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (board.blackBishops)
	{
		if (board.blackBishops & (board.blackBishops - 1))
		{
			score -= BONUS_BISHOP_PAIR;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL> BLACK BISHOP PAIR BONUS         " << BONUS_BISHOP_PAIR << std::endl;
			iblackpos -= BONUS_BISHOP_PAIR;
#endif              
		}
	}

	temp = board.blackBishops;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK BISHOP ON                 " << SQUARENAME[square] << std::endl;
#endif

		score -= BISHOPPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << BISHOPPOS_B[square] << std::endl;
		iblackpos -= BISHOPPOS_B[square];
#endif

		score -= BISHOP_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << BISHOP_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= BISHOP_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black rooks
	// - position on the board
	// - distance from opponent king
	// - on the same file as a passed pawn
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.blackRooks;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK ROOK ON                   " << SQUARENAME[square] << std::endl;
#endif

		score -= ROOKPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << ROOKPOS_B[square] << std::endl;
		iblackpos -= ROOKPOS_B[square];
#endif

		score -= ROOK_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << ROOK_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= ROOK_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		if (FILEMASK[square] & blackpassedpawns)
		{
			if ((unsigned int)square > firstOne(FILEMASK[square] & blackpassedpawns))
			{
				score -= BONUS_ROOK_BEHIND_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
				std::cout << "EVAL>  BEHIND PASSED PAWN BONUS       " << BONUS_ROOK_BEHIND_PASSED_PAWN << std::endl;
				iblackpos -= BONUS_ROOK_BEHIND_PASSED_PAWN;
#endif
			}
		}
		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black queens
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.blackQueens;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK QUEEN ON                  " << SQUARENAME[square] << std::endl;
#endif

		score -= QUEENPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << QUEENPOS_B[square] << std::endl;
		iblackpos -= QUEENPOS_B[square];
#endif

		score -= QUEEN_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << QUEEN_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= QUEEN_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		temp ^= BITSET[square];
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate the black king
	// - position on the board
	// - proximity to the pawns
	// - pawn shield (not in the endgame)
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (endgame)
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK KING ON (endgame)         " << SQUARENAME[blackkingsquare] << std::endl;
#endif

		score -= KINGPOS_ENDGAME_B[blackkingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_ENDGAME_B[blackkingsquare] << std::endl;
		iblackpos -= KINGPOS_ENDGAME_B[blackkingsquare];
		iblackking -= KINGPOS_ENDGAME_B[blackkingsquare];
#endif
	}
	else
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK KING ON (no endgame)      " << SQUARENAME[blackkingsquare] << std::endl;
#endif

		score -= KINGPOS_B[blackkingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_B[blackkingsquare] << std::endl;
		iblackpos -= KINGPOS_B[blackkingsquare];
		iblackking -= KINGPOS_B[blackkingsquare];
#endif

		// add pawn shield bonus if we're not in the endgame:
		// strong pawn shield bonus if the pawns are near the king:
		score -= BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  STRONG PAWN SHIELD SCORE IS    " << BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns) << std::endl;
		iblackpos -= BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns);
		iblackking -= BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns);
#endif

		// weaker pawn shield bonus if the pawns are not so near the king:
		score -= BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  WEAK PAWN SHIELD SCORE IS      " << BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns) << std::endl;
		iblackpos -= BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns);
		iblackking -= BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns);
#endif

	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Return the score
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> SUMMARY:" << std::endl;
	std::cout << "EVAL>  MATERIAL                :" << board.Material << std::endl;
	std::cout << "EVAL>  UNEVEN MATERIAL BONUS   :" << iexch << std::endl;
	std::cout << "EVAL>  WHITE POSITIONAL BONUS  :" << iwhitepos << std::endl;
	std::cout << "EVAL>     WHITE PAWNS BONUS    :" << iwhitepawns << std::endl;
	std::cout << "EVAL>     WHITE KING POS/SAF   :" << iwhiteking << std::endl;
	std::cout << "EVAL>  BLACK POSITIONAL BONUS  :" << iblackpos << std::endl;
	std::cout << "EVAL>     BLACK PAWNS BONUS    :" << iblackpawns << std::endl;
	std::cout << "EVAL>     BLACK KING POS/SAF   :" << iblackking << std::endl;
#endif

	if (board.nextMove) return -score;
	else return score;

}

void Board::mirror()
{
	// Mirrors the board position (NOTE: move buffers are not effected!!!).
	// The intended use is to test symmetry of the evalaution function.
	// Don't forget to mirror the position back to the original position, after testing the evaluation function.

	int i, mirroredBoard[64];
	unsigned char nextMirror;
	int fiftyMMirror, castleWMirror, castleBMirror, epSqMirror;
	int kmoveBufLen, kendOfGame, kendOfSearch;

	nextMirror = !nextMove;
	fiftyMMirror = movesSincePawn;
	castleWMirror = castleBlack;
	castleBMirror = castleWhite;
	if (epSquare)
	{
		if (epSquare < 24) epSqMirror = epSquare + 24;
		else epSqMirror = epSquare - 24;
	}

	for (i = 0; i < 64; i++)
	{
		mirroredBoard[i] = square[MIRROR[i]];
		// swap piece color (4th bit):
		if (mirroredBoard[i] != EMPTY) mirroredBoard[i] ^= 8;
	}

	// keep the move buffer intact:
	kmoveBufLen = moveBufLen[0];
	kendOfGame = endOfGame;
	kendOfSearch = endOfSearch;
	initFromSquares(mirroredBoard, nextMirror, fiftyMMirror, castleWMirror, castleBMirror, epSqMirror);
	moveBufLen[0] = kmoveBufLen;
	endOfGame = kendOfGame;
	endOfSearch = kendOfSearch;

	return;
}