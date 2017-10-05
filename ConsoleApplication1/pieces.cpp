#include "pieces.h"
#include "board.h"
#include <iostream>

// Identifier of next move:
extern const unsigned char WHITE_MOVE = 0;
extern const unsigned char BLACK_MOVE = 1;

// Piece identifiers, 4 bits each.
// Useful bitwise properties of this numbering scheme:
// white = 0..., black = 1..., sliding = .1.., nonsliding = .0..
// rank/file sliding pieces = .11., diagonally sliding pieces = .1.1
// pawns and kings (without color bits), are < 3
// major pieces (without color bits set), are > 5
// minor and major pieces (without color bits set), are > 2
extern const unsigned char EMPTY = 0;                //  0000
extern const unsigned char WHITE_PAWN = 1;           //  0001
extern const unsigned char WHITE_KING = 2;           //  0010
extern const unsigned char WHITE_KNIGHT = 3;         //  0011
extern const unsigned char WHITE_BISHOP = 5;         //  0101
extern const unsigned char WHITE_ROOK = 6;           //  0110
extern const unsigned char WHITE_QUEEN = 7;          //  0111
extern const unsigned char BLACK_PAWN = 9;           //  1001
extern const unsigned char BLACK_KING = 10;          //  1010
extern const unsigned char BLACK_KNIGHT = 11;        //  1011
extern const unsigned char BLACK_BISHOP = 13;        //  1101
extern const unsigned char BLACK_ROOK = 14;          //  1110
extern const unsigned char BLACK_QUEEN = 15;         //  1111

const char* PIECENAMES[16] = { "  ","P ","K ","N ","  ","B ","R ","Q ",
"  ","P*","K*","N*","  ","B*","R*","Q*" };

// Value of material, in centipawns:
extern const int PAWN_VALUE = 100;
extern const int KNIGHT_VALUE = 300;
extern const int BISHOP_VALUE = 300;
extern const int ROOK_VALUE = 500;
extern const int QUEEN_VALUE = 900;
extern const int KING_VALUE = 9999;
extern const int CHECK_MATE = KING_VALUE;

// Used for castling:
unsigned char CANCASTLEOO = 1;
unsigned char CANCASTLEOOO = 2;
unsigned int WHITE_OOO_CASTL;
unsigned int BLACK_OOO_CASTL;
unsigned int WHITE_OO_CASTL;
unsigned int BLACK_OO_CASTL;

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

int PAWN_OWN_DISTANCE[8] = { 0,   8,  4,  2,  0,  0,  0,  0 };
int PAWN_OPPONENT_DISTANCE[8] = { 0,   2,  1,  0,  0,  0,  0,  0 };

int PENALTY_DOUBLED_PAWN = 10;
int PENALTY_ISOLATED_PAWN = 20;
int PENALTY_BACKWARD_PAWN = 8;
int BONUS_PASSED_PAWN = 20;

void Knight::generateMoves() {
	BitMap target = ~board.whitePieces; // possible moves are any squares besides those occupied by the white pieces
	BitMap tempPiece = board.whiteKnights;

	unsigned int from, to, capt;
	int i = 0;
	while (tempPiece) {
		from = ls1b(tempPiece);
		BitMap tempMove = attacks[from] & target;
		while (tempMove) {
			to = ls1b(tempMove);
			capt = board.square[to];

			Move move;
			move.set(WHITE_KNIGHT, capt, from, to, 0);
			moves[i] = move; // This will likely need refactored later
			i++;

			tempMove ^= BITSET[to]; // Rename BITSET to something more intuitive
		}
		tempPiece ^= BITSET[from];
	}
}

Pawn::Pawn() {
	int file, rank, afile, arank, square;
	for (square = 0; square < 64; square++) {
		WHITE_PAWN_ATTACKS[square] = 0x0;
		WHITE_PAWN_MOVES[square] = 0x0;
		WHITE_PAWN_DOUBLE_MOVES[square] = 0x0;
		BLACK_PAWN_ATTACKS[square] = 0x0;
		BLACK_PAWN_MOVES[square] = 0x0;
		BLACK_PAWN_DOUBLE_MOVES[square] = 0x0;
	}

	// WHITE_PAWN_ATTACKS
	for (square = 0; square < 64; square++) {
		file = FILES[square]; rank = RANKS[square];
		afile = file - 1; arank = rank + 1;
		if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
			WHITE_PAWN_ATTACKS[square] |= BITSET[BOARDINDEX[afile][arank]];
		afile = file + 1; arank = rank + 1;
		if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
			WHITE_PAWN_ATTACKS[square] |= BITSET[BOARDINDEX[afile][arank]];
	}

	// WHITE_PAWN_MOVES
	for (square = 0; square < 64; square++) {
		file = FILES[square]; rank = RANKS[square];
		afile = file; arank = rank + 1;
		if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
			WHITE_PAWN_MOVES[square] |= BITSET[BOARDINDEX[afile][arank]];
		if (rank == 2) {
			afile = file; arank = rank + 2;
			if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
				WHITE_PAWN_DOUBLE_MOVES[square] |= BITSET[BOARDINDEX[afile][arank]];
		}
	}

	// BLACK_PAWN_ATTACKS
	for (square = 0; square < 64; square++) {
		file = FILES[square]; rank = RANKS[square];
		afile = file - 1; arank = rank - 1;
		if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
			BLACK_PAWN_ATTACKS[square] |= BITSET[BOARDINDEX[afile][arank]];
		afile = file + 1; arank = rank - 1;
		if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
			BLACK_PAWN_ATTACKS[square] |= BITSET[BOARDINDEX[afile][arank]];
	}

	// BLACK_PAWN_MOVES
	for (square = 0; square < 64; square++) {
		file = FILES[square]; rank = RANKS[square];
		afile = file; arank = rank - 1;
		if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
			BLACK_PAWN_MOVES[square] |= BITSET[BOARDINDEX[afile][arank]];
		if (rank == 7)
		{
			afile = file; arank = rank - 2;
			if ((afile >= 1) & (afile <= 8) & (arank >= 1) & (arank <= 8))
				BLACK_PAWN_DOUBLE_MOVES[square] |= BITSET[BOARDINDEX[afile][arank]];
		}
	}
}

void Pawn::generateMoves(unsigned char color, int* moveBufIndex, BitMap freeSquares) {
	Move move;
	
	if (color == BLACK_MOVE) {
		move.setPiec(BLACK_PAWN);
		BitMap tempPiece = board.blackPawns;
		while (tempPiece) {
			unsigned int from = ls1b(tempPiece);
			move.setFrom(from);
			BitMap tempMove = BLACK_PAWN_MOVES[from] & freeSquares;                // normal moves
			if (RANKS[from] == 7 && tempMove)
				tempMove |= (BLACK_PAWN_DOUBLE_MOVES[from] & freeSquares);  // add double moves
			tempMove |= BLACK_PAWN_ATTACKS[from] & board.whitePieces;       // add captures
			while (tempMove) {
				unsigned int to = ls1b(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				// add promotions
				if ((RANKS[to]) == 1) {
					move.setProm(BLACK_QUEEN);   board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(BLACK_ROOK);    board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(BLACK_BISHOP);  board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(BLACK_KNIGHT);  board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(EMPTY);
				}
				else
					board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			// add en-passant captures:
			// do a quick check first
			if (board.epSquare) {
				if (BLACK_PAWN_ATTACKS[from] & BITSET[board.epSquare]) {
					move.setProm(BLACK_PAWN);
					move.setCapt(WHITE_PAWN);
					move.setTosq(board.epSquare);
					board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
				}
			}
			tempPiece ^= BITSET[from];
			move.setProm(EMPTY);
		}
	}
	else if (color == WHITE_MOVE) {
		move.setPiec(WHITE_PAWN);
		BitMap tempPiece = board.whitePawns;
		while (tempPiece) {
			unsigned int from = ls1b(tempPiece);
			move.setFrom(from);
			BitMap tempMove = WHITE_PAWN_MOVES[from] & freeSquares;                // normal moves
			if (RANKS[from] == 2 && tempMove)
				tempMove |= (WHITE_PAWN_DOUBLE_MOVES[from] & freeSquares);  // add double moves
			tempMove |= WHITE_PAWN_ATTACKS[from] & board.blackPieces;       // add captures
			while (tempMove) {
				unsigned int to = ls1b(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				// add promotions
				if ((RANKS[to]) == 8) {
					move.setProm(WHITE_QUEEN);   board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(WHITE_ROOK);    board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(WHITE_BISHOP);  board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(WHITE_KNIGHT);  board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
					move.setProm(EMPTY);
				}
				else
					board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			// add en-passant captures:
			// do a quick check first
			if (board.epSquare) {
				if (WHITE_PAWN_ATTACKS[from] & BITSET[board.epSquare]) {
					move.setProm(WHITE_PAWN);
					move.setCapt(BLACK_PAWN);
					move.setTosq(board.epSquare);
					board.moveBuffer[(*moveBufIndex)++].bitfield = move.bitfield;
				}
			}
			tempPiece ^= BITSET[from];
			move.setProm(EMPTY);
		}
	}
}

void Pawn::eval(BitMap* whitepassedpawns, int score, int DISTANCE[64][64], BitMap PASSED_WHITE[64], BitMap ISOLATED_WHITE[64],
	            BitMap BACKWARD_WHITE[64], int whitekingsquare, int blackkingsquare, bool endgame) {
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white pawns
	// - position on the board
	// - distance from opponent king
	// - distance from own king
	// - passed, doubled, isolated or backward pawns
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	BitMap temp = board.whitePawns;
	int square;
	while (temp) {
		square = ls1b(temp);

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
			*whitepassedpawns ^= BITSET[square];
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
}