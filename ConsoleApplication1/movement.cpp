#include "movement.h"
#include "bitops.h"
#include "pieces.h"
#include "board.h"

// Macro's to define sliding attacks:
#define RANKMOVES(a)       (RANK_ATTACKS[(a)][((board.occupiedSquares & RANKMASK[(a)]) >> RANKSHIFT[(a)])] & targetBitmap)
#define FILEMOVES(a)       (FILE_ATTACKS[(a)][((board.occupiedSquares & FILEMASK[(a)]) * FILEMAGIC[(a)]) >> 57] & targetBitmap)
#define SLIDEA8H1MOVES(a)  (DIAGA8H1_ATTACKS[(a)][((board.occupiedSquares & DIAGA8H1MASK[(a)]) * DIAGA8H1MAGIC[(a)]) >> 57] & targetBitmap)
#define SLIDEA1H8MOVES(a)  (DIAGA1H8_ATTACKS[(a)][((board.occupiedSquares & DIAGA1H8MASK[(a)]) * DIAGA1H8MAGIC[(a)]) >> 57] & targetBitmap)
#define ROOKMOVES(a)       (RANKMOVES(a) | FILEMOVES(a))
#define BISHOPMOVES(a)     (SLIDEA8H1MOVES(a) | SLIDEA1H8MOVES(a))
#define QUEENMOVES(a)      (BISHOPMOVES(a) | ROOKMOVES(a))

// Attack tables:
BitMap WHITE_PAWN_ATTACKS[64];
BitMap WHITE_PAWN_MOVES[64];
BitMap WHITE_PAWN_DOUBLE_MOVES[64];
BitMap BLACK_PAWN_ATTACKS[64];
BitMap BLACK_PAWN_MOVES[64];
BitMap BLACK_PAWN_DOUBLE_MOVES[64];
BitMap KNIGHT_ATTACKS[64];
BitMap KING_ATTACKS[64];
BitMap RANK_ATTACKS[64][64];      // 32KB
BitMap FILE_ATTACKS[64][64];      // 32KB
BitMap DIAGA8H1_ATTACKS[64][64];  // 32KB
BitMap DIAGA1H8_ATTACKS[64][64];  // 32KB

								  // Move generator shift for ranks:
extern const int RANKSHIFT[64] = {
	1,  1,  1,  1,  1,  1,  1,  1,
	9,  9,  9,  9,  9,  9,  9,  9,
	17, 17, 17, 17, 17, 17, 17, 17,
	25, 25, 25, 25, 25, 25, 25, 25,
	33, 33, 33, 33, 33, 33, 33, 33,
	41, 41, 41, 41, 41, 41, 41, 41,
	49, 49, 49, 49, 49, 49, 49, 49,
	57, 57, 57, 57, 57, 57, 57, 57
};

// Move generator magic multiplication numbers for files:
extern const BitMap _FILEMAGICS[8] = {
	0x8040201008040200,
	0x4020100804020100,
	0x2010080402010080,
	0x1008040201008040,
	0x0804020100804020,
	0x0402010080402010,
	0x0201008040201008,
	0x0100804020100804
};

// Move generator magic multiplication numbers for diagonals:
extern const BitMap _DIAGA8H1MAGICS[15] = {
	0x0,
	0x0,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0080808080808080,
	0x0040404040404040,
	0x0020202020202020,
	0x0010101010101010,
	0x0008080808080808,
	0x0,
	0x0
};

// Move generator magic multiplication numbers for diagonals:
extern const BitMap _DIAGA1H8MAGICS[15] = {
	0x0,
	0x0,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x8080808080808000,
	0x4040404040400000,
	0x2020202020000000,
	0x1010101000000000,
	0x0808080000000000,
	0x0,
	0x0
};

// Move generator 6-bit masking and magic multiplication numbers:
BitMap RANKMASK[64];
BitMap FILEMASK[64];
BitMap FILEMAGIC[64];
BitMap DIAGA8H1MASK[64];
BitMap DIAGA8H1MAGIC[64];
BitMap DIAGA1H8MASK[64];
BitMap DIAGA1H8MAGIC[64];

// We use one generalized sliding attacks array: [8 squares][64 states]
// the unsigned char (=8 bits) contains the attacks for a rank, file or diagonal
unsigned char GEN_SLIDING_ATTACKS[8][64];

// Used for castling:
BitMap maskEG[2];
BitMap maskFG[2];
BitMap maskBD[2];
BitMap maskCE[2];

int ICAPT;
int IEP;
int IPROM;
int ICASTLOO;
int ICASTLOOO;
int ICHECK;

void Move::set(unsigned int piece, unsigned int captured, unsigned int from, unsigned int to, unsigned int promote) {
	// from (6 bits)
	// tosq (6 bits)
	// piec (4 bits)
	// capt (4 bits)
	// prom (4 bits)

	// Set piece
	bitfield &= 0xffff0fff;
	bitfield |= (piece & 0x0000000f) << 12;
	
	// Set captured
	bitfield &= 0xfff0ffff;
	bitfield |= (captured & 0x0000000f) << 16;

	// Set from
	bitfield &= 0xffffffc0;
	bitfield |= (from & 0x0000003f);

	// Set to
	bitfield &= 0xfffff03f;
	bitfield |= (to & 0x0000003f) << 6;

	// Set promotion
	bitfield &= 0xff0fffff;
	bitfield |= (promote & 0x0000000f) << 20;
}

void Move::clear() {
	bitfield = 0;
}

void Move::setFrom(unsigned int from)
{   // bits  0.. 5
	bitfield &= 0xffffffc0; bitfield |= (from & 0x0000003f);
}

void Move::setTosq(unsigned int tosq)
{   // bits  6..11      
	bitfield &= 0xfffff03f; bitfield |= (tosq & 0x0000003f) << 6;
}

void Move::setPiec(unsigned int piec)
{   // bits 12..15
	bitfield &= 0xffff0fff; bitfield |= (piec & 0x0000000f) << 12;
}

void Move::setCapt(unsigned int capt)
{   // bits 16..19
	bitfield &= 0xfff0ffff; bitfield |= (capt & 0x0000000f) << 16;
}

void Move::setProm(unsigned int prom)
{   // bits 20..23
	bitfield &= 0xff0fffff; bitfield |= (prom & 0x0000000f) << 20;
}

// read move information:
// first shift right, then mask to get the info

unsigned int Move::getFrom()
{   // 6 bits (value 0..63), position  0.. 5
	return (bitfield & 0x0000003f);
}

unsigned int Move::getTosq()
{   // 6 bits (value 0..63), position  6..11
	return (bitfield >> 6) & 0x0000003f;
}

unsigned int Move::getPiec()
{   // 4 bits (value 0..15), position 12..15
	return (bitfield >> 12) & 0x0000000f;
}

unsigned int Move::getCapt()
{   // 4 bits (value 0..15), position 16..19
	return (bitfield >> 16) & 0x0000000f;
}

unsigned int Move::getProm()
{   // 4 bits (value 0..15), position 20..23
	return (bitfield >> 20) & 0x0000000f;
}

// boolean checks for some types of moves.
// first mask, then compare
// Note that we are using the bit-wise properties of piece identifiers, so we cannot just change them anymore !

bool Move::isWhitemove()
{   // piec is white: bit 15 must be 0
	return (~bitfield & 0x00008000) == 0x00008000;
}

bool Move::isBlackmove()
{   // piec is black: bit 15 must be 1
	return (bitfield & 0x00008000) == 0x00008000;
}

bool Move::isCapture()
{   // capt is nonzero, bits 16 to 19 must be nonzero
	return (bitfield & 0x000f0000) != 0x00000000;
}

bool Move::isKingcaptured()
{   // bits 17 to 19 must be 010
	return (bitfield & 0x00070000) == 0x00020000;
}

bool Move::isRookmove()
{   // bits 13 to 15 must be 110
	return (bitfield & 0x00007000) == 0x00006000;
}

bool Move::isRookcaptured()
{   // bits 17 to 19 must be 110
	return (bitfield & 0x00070000) == 0x00060000;
}

bool Move::isKingmove()
{   // bits 13 to 15 must be 010
	return (bitfield & 0x00007000) == 0x00002000;
}

bool Move::isPawnmove()
{   // bits 13 to 15 must be 001
	return (bitfield & 0x00007000) == 0x00001000;
}

bool Move::isPawnDoublemove()
{   // bits 13 to 15 must be 001 &
	//     bits 4 to 6 must be 001 (from rank 2) & bits 10 to 12 must be 011 (to rank 4)
	// OR: bits 4 to 6 must be 110 (from rank 7) & bits 10 to 12 must be 100 (to rank 5)

	return (((bitfield & 0x00007000) == 0x00001000) && ((((bitfield & 0x00000038) == 0x00000008) && (((bitfield & 0x00000e00) == 0x00000600))) ||
		(((bitfield & 0x00000038) == 0x00000030) && (((bitfield & 0x00000e00) == 0x00000800)))));
}

bool Move::isEnpassant()
{   // prom is a pawn, bits 21 to 23 must be 001
	return (bitfield & 0x00700000) == 0x00100000;
}

bool Move::isPromotion()
{   // prom (with color bit removed), .xxx > 2 (not king or pawn)
	return (bitfield & 0x00700000) >  0x00200000;
}

bool Move::isCastle()
{   // prom is a king, bits 21 to 23 must be 010
	return (bitfield & 0x00700000) == 0x00200000;
}

bool Move::isCastleOO()
{   // prom is a king and tosq is on the g-file
	return (bitfield & 0x007001c0) == 0x00200180;
}

bool Move::isCastleOOO()
{   // prom is a king and tosq is on the c-file
	return (bitfield & 0x007001c0) == 0x00200080;
}

int movegen(int index)
{

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// This is winglet's pseudo-legal bitmap move generator,
	// using magic multiplication instead of rotated bitboards.
	// There is no check if a move leaves the king in check
	// The first free location in moveBuffer[] is supplied in index,
	// and the new first free location is returned
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	unsigned char opponentSide;
	unsigned int from, to;
	BitMap tempPiece, tempMove;
	BitMap targetBitmap, freeSquares;
	Move move;

	move.clear();
	opponentSide = !board.nextMove;
	freeSquares = ~board.occupiedSquares;

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Black to move
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (board.nextMove) // black to move
	{
		targetBitmap = ~board.blackPieces; // we cannot capture one of our own pieces!
		
		// Black Pawns
		Pawn pawn;
		pawn.generateMoves(BLACK_MOVE, index, freeSquares);

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Black Knights
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(BLACK_KNIGHT);
		tempPiece = board.blackKnights;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = KNIGHT_ATTACKS[from] & targetBitmap;
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Black Bishops
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(BLACK_BISHOP);
		tempPiece = board.blackBishops;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = BISHOPMOVES(from);   // see Macro's
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Black Rooks
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(BLACK_ROOK);
		tempPiece = board.blackRooks;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = ROOKMOVES(from);   // see Macro's
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Black Queens
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(BLACK_QUEEN);
		tempPiece = board.blackQueens;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = QUEENMOVES(from);   // see Macro's
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Black King
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(BLACK_KING);
		tempPiece = board.blackKing;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = KING_ATTACKS[from] & targetBitmap;
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}

			//     Black 0-0 Castling:
			if (board.castleBlack & CANCASTLEOO)
			{
				if (!(maskFG[1] & board.occupiedSquares))
				{
					if (!isAttacked(maskEG[BLACK_MOVE], WHITE_MOVE))
					{
						board.moveBuffer[index++].bitfield = BLACK_OO_CASTL;   // predefined unsigned int
					}
				}
			}
			//     Black 0-0-0 Castling:
			if (board.castleBlack & CANCASTLEOOO)
			{
				if (!(maskBD[1] & board.occupiedSquares))
				{
					if (!isAttacked(maskCE[BLACK_MOVE], WHITE_MOVE))
					{
						board.moveBuffer[index++].bitfield = BLACK_OOO_CASTL; // predefined unsigned int
					}
				}
			}
			tempPiece ^= BITSET[from];
			move.setProm(EMPTY);
		}
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// White to move
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	else
	{
		targetBitmap = ~board.whitePieces; // we cannot capture one of our own pieces!

	    // White Pawns
		Pawn pawn;
		pawn.generateMoves(WHITE_MOVE, index, freeSquares);

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// White Knights
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(WHITE_KNIGHT);
		tempPiece = board.whiteKnights;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = KNIGHT_ATTACKS[from] & targetBitmap;
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// White Bishops
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(WHITE_BISHOP);
		tempPiece = board.whiteBishops;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = BISHOPMOVES(from);   // see Macro's
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// White Rooks
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(WHITE_ROOK);
		tempPiece = board.whiteRooks;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = ROOKMOVES(from);   // see Macro's
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// White Queens
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(WHITE_QUEEN);
		tempPiece = board.whiteQueens;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = QUEENMOVES(from);   // see Macro's
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}
			tempPiece ^= BITSET[from];
		}

		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// White king
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(WHITE_KING);
		tempPiece = board.whiteKing;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = KING_ATTACKS[from] & targetBitmap;
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				board.moveBuffer[index++].bitfield = move.bitfield;
				tempMove ^= BITSET[to];
			}

			//     White 0-0 Castling:
			if (board.castleWhite & CANCASTLEOO)
			{
				if (!(maskFG[0] & board.occupiedSquares))
				{
					if (!isAttacked(maskEG[WHITE_MOVE], BLACK_MOVE))
					{
						board.moveBuffer[index++].bitfield = WHITE_OO_CASTL; // predefined unsigned int
					}
				}
			}

			//     White 0-0-0 Castling:
			if (board.castleWhite & CANCASTLEOOO)
			{
				if (!(maskBD[0] & board.occupiedSquares))
				{
					if (!isAttacked(maskCE[WHITE_MOVE], BLACK_MOVE))
					{
						board.moveBuffer[index++].bitfield = WHITE_OOO_CASTL; // predefined unsigned int
					}
				}
			}
			tempPiece ^= BITSET[from];
			move.setProm(EMPTY);
		}
	}
	return index;
}

bool isAttacked(BitMap &targetBitmap, const unsigned char &fromSide)
{

	//     ===========================================================================
	//     isAttacked is used mainly as a move legality test to see if targetBitmap is
	//     attacked by white or black.
	//  Returns true at the first attack found, and returns false if no attack is found.
	//  It can be used for:
	//   - check detection, and
	//   - castling legality: test to see if the king passes through, or ends up on,
	//     a square that is attacked
	//     ===========================================================================

	BitMap tempTarget;
	BitMap slidingAttackers;
	int to;

	tempTarget = targetBitmap;
	if (fromSide) // test for attacks from BLACK to targetBitmap
	{
		while (tempTarget)
		{
			to = firstOne(tempTarget);

			if (board.blackPawns & WHITE_PAWN_ATTACKS[to]) return true;
			if (board.blackKnights & KNIGHT_ATTACKS[to]) return true;
			if (board.blackKing & KING_ATTACKS[to]) return true;

			// file / rank attacks
			slidingAttackers = board.blackQueens | board.blackRooks;
			if (slidingAttackers)
			{
				if (RANK_ATTACKS[to][((board.occupiedSquares & RANKMASK[to]) >> RANKSHIFT[to])] & slidingAttackers) return true;
				if (FILE_ATTACKS[to][((board.occupiedSquares & FILEMASK[to]) * FILEMAGIC[to]) >> 57] & slidingAttackers) return true;
			}

			// diagonals
			slidingAttackers = board.blackQueens | board.blackBishops;
			if (slidingAttackers)
			{
				if (DIAGA8H1_ATTACKS[to][((board.occupiedSquares & DIAGA8H1MASK[to]) * DIAGA8H1MAGIC[to]) >> 57] & slidingAttackers) return true;
				if (DIAGA1H8_ATTACKS[to][((board.occupiedSquares & DIAGA1H8MASK[to]) * DIAGA1H8MAGIC[to]) >> 57] & slidingAttackers) return true;
			}

			tempTarget ^= BITSET[to];
		}
	}
	else // test for attacks from WHITE to targetBitmap
	{
		while (tempTarget)
		{
			to = firstOne(tempTarget);

			if (board.whitePawns & BLACK_PAWN_ATTACKS[to]) return true;
			if (board.whiteKnights & KNIGHT_ATTACKS[to]) return true;
			if (board.whiteKing & KING_ATTACKS[to]) return true;

			// file / rank attacks
			slidingAttackers = board.whiteQueens | board.whiteRooks;
			if (slidingAttackers)
			{
				if (RANK_ATTACKS[to][((board.occupiedSquares & RANKMASK[to]) >> RANKSHIFT[to])] & slidingAttackers) return true;
				if (FILE_ATTACKS[to][((board.occupiedSquares & FILEMASK[to]) * FILEMAGIC[to]) >> 57] & slidingAttackers) return true;
			}

			// diagonals:
			slidingAttackers = board.whiteQueens | board.whiteBishops;
			if (slidingAttackers)
			{
				if (DIAGA8H1_ATTACKS[to][((board.occupiedSquares & DIAGA8H1MASK[to]) * DIAGA8H1MAGIC[to]) >> 57] & slidingAttackers) return true;
				if (DIAGA1H8_ATTACKS[to][((board.occupiedSquares & DIAGA1H8MASK[to]) * DIAGA1H8MAGIC[to]) >> 57] & slidingAttackers) return true;
			}

			tempTarget ^= BITSET[to];
		}
	}
	return false;
}

#ifdef WINGLET_DEBUG_MOVES
void debugMoves(char *, Move &);
#endif

void makeMove(Move &move)
{
	unsigned int from = move.getFrom();
	unsigned int to = move.getTosq();
	unsigned int piece = move.getPiec();
	unsigned int captured = move.getCapt();

	BitMap fromBitMap = BITSET[from];
	BitMap fromToBitMap = fromBitMap | BITSET[to];

	board.gameLine[board.endOfSearch].move.bitfield = move.bitfield;
	board.gameLine[board.endOfSearch].castleWhite = board.castleWhite;
	board.gameLine[board.endOfSearch].castleBlack = board.castleBlack;
	board.gameLine[board.endOfSearch].fiftyMove = board.fiftyMove;
	board.gameLine[board.endOfSearch].epSquare = board.epSquare;
	board.endOfSearch++;

	switch (piece)
	{
	case 1: // white pawn:
		board.whitePawns ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = WHITE_PAWN;
		board.epSquare = 0;
		board.fiftyMove = 0;
		if (RANKS[from] == 2)
			if (RANKS[to] == 4) board.epSquare = from + 8;
		if (captured)
		{
			if (move.isEnpassant())
			{
				board.blackPawns ^= BITSET[to - 8];
				board.blackPieces ^= BITSET[to - 8];
				board.occupiedSquares ^= fromToBitMap | BITSET[to - 8];
				board.square[to - 8] = EMPTY;
				board.Material += PAWN_VALUE;
			}
			else
			{
				makeCapture(captured, to);
				board.occupiedSquares ^= fromBitMap;
			}
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isPromotion())
		{
			makeWhitePromotion(move.getProm(), to);
			board.square[to] = move.getProm();
		}
		break;

	case 2: // white king:
		board.whiteKing ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = WHITE_KING;
		board.epSquare = 0;
		board.fiftyMove++;
		board.castleWhite = 0;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isCastle())
		{
			if (move.isCastleOO())
			{
				board.whiteRooks ^= BITSET[H1] | BITSET[F1];
				board.whitePieces ^= BITSET[H1] | BITSET[F1];
				board.occupiedSquares ^= BITSET[H1] | BITSET[F1];
				board.square[H1] = EMPTY;
				board.square[F1] = WHITE_ROOK;
			}
			else
			{
				board.whiteRooks ^= BITSET[A1] | BITSET[D1];
				board.whitePieces ^= BITSET[A1] | BITSET[D1];
				board.occupiedSquares ^= BITSET[A1] | BITSET[D1];
				board.square[A1] = EMPTY;
				board.square[D1] = WHITE_ROOK;
			}
		}
		break;

	case 3: // white knight:
		board.whiteKnights ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = WHITE_KNIGHT;
		board.epSquare = 0;
		board.fiftyMove++;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 5: // white bishop:
		board.whiteBishops ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = WHITE_BISHOP;
		board.epSquare = 0;
		board.fiftyMove++;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 6: // white rook:
		board.whiteRooks ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = WHITE_ROOK;
		board.epSquare = 0;
		board.fiftyMove++;
		if (from == A1) board.castleWhite &= ~CANCASTLEOOO;
		if (from == H1) board.castleWhite &= ~CANCASTLEOO;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 7: // white queen:
		board.whiteQueens ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = WHITE_QUEEN;
		board.epSquare = 0;
		board.fiftyMove++;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 9: // black pawn:
		board.blackPawns ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = BLACK_PAWN;
		board.epSquare = 0;
		board.fiftyMove = 0;
		if (RANKS[from] == 7)
			if (RANKS[to] == 5) board.epSquare = from - 8;
		if (captured)
		{
			if (move.isEnpassant())
			{
				board.whitePawns ^= BITSET[to + 8];
				board.whitePieces ^= BITSET[to + 8];
				board.occupiedSquares ^= fromToBitMap | BITSET[to + 8];
				board.square[to + 8] = EMPTY;
				board.Material -= PAWN_VALUE;
			}
			else
			{
				makeCapture(captured, to);
				board.occupiedSquares ^= fromBitMap;
			}
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isPromotion())
		{
			makeBlackPromotion(move.getProm(), to);
			board.square[to] = move.getProm();
		}
		break;

	case 10: // black king:
		board.blackKing ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = BLACK_KING;
		board.epSquare = 0;
		board.fiftyMove++;
		board.castleBlack = 0;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isCastle())
		{
			if (move.isCastleOO())
			{
				board.blackRooks ^= BITSET[H8] | BITSET[F8];
				board.blackPieces ^= BITSET[H8] | BITSET[F8];
				board.occupiedSquares ^= BITSET[H8] | BITSET[F8];
				board.square[H8] = EMPTY;
				board.square[F8] = BLACK_ROOK;
			}
			else
			{
				board.blackRooks ^= BITSET[A8] | BITSET[D8];
				board.blackPieces ^= BITSET[A8] | BITSET[D8];
				board.occupiedSquares ^= BITSET[A8] | BITSET[D8];
				board.square[A8] = EMPTY;
				board.square[D8] = BLACK_ROOK;
			}
		}
		break;

	case 11: // black knight:
		board.blackKnights ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = BLACK_KNIGHT;
		board.epSquare = 0;
		board.fiftyMove++;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 13: // black bishop:
		board.blackBishops ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = BLACK_BISHOP;
		board.epSquare = 0;
		board.fiftyMove++;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 14: // black rook:
		board.blackRooks ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = BLACK_ROOK;
		board.epSquare = 0;
		board.fiftyMove++;
		if (from == A8) board.castleBlack &= ~CANCASTLEOOO;
		if (from == H8) board.castleBlack &= ~CANCASTLEOO;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 15: // black queen:
		board.blackQueens ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = EMPTY;
		board.square[to] = BLACK_QUEEN;
		board.epSquare = 0;
		board.fiftyMove++;
		if (captured)
		{
			makeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;
	}
	board.nextMove = !board.nextMove;

#ifdef WINGLET_DEBUG_MOVES
	debugMoves("makemove", move);
#endif

}

void unmakeMove(Move &move)
{
	unsigned int piece = move.getPiec();
	unsigned int captured = move.getCapt();
	unsigned int from = move.getFrom();
	unsigned int to = move.getTosq();

	BitMap fromBitMap = BITSET[from];
	BitMap fromToBitMap = fromBitMap | BITSET[to];

	switch (piece)
	{
	case 1: // white pawn:
		board.whitePawns ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = WHITE_PAWN;
		board.square[to] = EMPTY;
		if (captured)
		{
			if (move.isEnpassant())
			{
				board.blackPawns ^= BITSET[to - 8];
				board.blackPieces ^= BITSET[to - 8];
				board.occupiedSquares ^= fromToBitMap | BITSET[to - 8];
				board.square[to - 8] = BLACK_PAWN;
				board.Material -= PAWN_VALUE;
			}
			else
			{
				unmakeCapture(captured, to);
				board.occupiedSquares ^= fromBitMap;
			}
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isPromotion())
		{
			unmakeWhitePromotion(move.getProm(), to);
		}
		break;

	case 2: // white king:
		board.whiteKing ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = WHITE_KING;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isCastle())
		{
			if (move.isCastleOO())
			{
				board.whiteRooks ^= BITSET[H1] | BITSET[F1];
				board.whitePieces ^= BITSET[H1] | BITSET[F1];
				board.occupiedSquares ^= BITSET[H1] | BITSET[F1];
				board.square[H1] = WHITE_ROOK;
				board.square[F1] = EMPTY;
			}
			else
			{
				board.whiteRooks ^= BITSET[A1] | BITSET[D1];
				board.whitePieces ^= BITSET[A1] | BITSET[D1];
				board.occupiedSquares ^= BITSET[A1] | BITSET[D1];
				board.square[A1] = WHITE_ROOK;
				board.square[D1] = EMPTY;
			}
		}
		break;

	case 3: // white knight:
		board.whiteKnights ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = WHITE_KNIGHT;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 5: // white bishop:
		board.whiteBishops ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = WHITE_BISHOP;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;

		break;

	case 6: // white rook:
		board.whiteRooks ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = WHITE_ROOK;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 7: // white queen:
		board.whiteQueens ^= fromToBitMap;
		board.whitePieces ^= fromToBitMap;
		board.square[from] = WHITE_QUEEN;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 9: // black pawn:
		board.blackPawns ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = BLACK_PAWN;
		board.square[to] = EMPTY;
		if (captured)
		{
			if (move.isEnpassant())
			{
				board.whitePawns ^= BITSET[to + 8];
				board.whitePieces ^= BITSET[to + 8];
				board.occupiedSquares ^= fromToBitMap | BITSET[to + 8];
				board.square[to + 8] = WHITE_PAWN;
				board.Material += PAWN_VALUE;
			}
			else
			{
				unmakeCapture(captured, to);
				board.occupiedSquares ^= fromBitMap;
			}
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isPromotion())
		{
			unmakeBlackPromotion(move.getProm(), to);
		}
		break;

	case 10: // black king:
		board.blackKing ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = BLACK_KING;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;

		if (move.isCastle())
		{
			if (move.isCastleOO())
			{
				board.blackRooks ^= BITSET[H8] | BITSET[F8];
				board.blackPieces ^= BITSET[H8] | BITSET[F8];
				board.occupiedSquares ^= BITSET[H8] | BITSET[F8];
				board.square[H8] = BLACK_ROOK;
				board.square[F8] = EMPTY;
			}
			else
			{
				board.blackRooks ^= BITSET[A8] | BITSET[D8];
				board.blackPieces ^= BITSET[A8] | BITSET[D8];
				board.occupiedSquares ^= BITSET[A8] | BITSET[D8];
				board.square[A8] = BLACK_ROOK;
				board.square[D8] = EMPTY;
			}
		}
		break;

	case 11: // black knight:
		board.blackKnights ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = BLACK_KNIGHT;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 13: // black bishop:
		board.blackBishops ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = BLACK_BISHOP;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 14: // black rook:
		board.blackRooks ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = BLACK_ROOK;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;

	case 15: // black queen:
		board.blackQueens ^= fromToBitMap;
		board.blackPieces ^= fromToBitMap;
		board.square[from] = BLACK_QUEEN;
		board.square[to] = EMPTY;
		if (captured)
		{
			unmakeCapture(captured, to);
			board.occupiedSquares ^= fromBitMap;
		}
		else board.occupiedSquares ^= fromToBitMap;
		break;
	}

	board.endOfSearch--;
	board.castleWhite = board.gameLine[board.endOfSearch].castleWhite;
	board.castleBlack = board.gameLine[board.endOfSearch].castleBlack;
	board.epSquare = board.gameLine[board.endOfSearch].epSquare;
	board.fiftyMove = board.gameLine[board.endOfSearch].fiftyMove;

	board.nextMove = !board.nextMove;

#ifdef WINGLET_DEBUG_MOVES
	debugMoves("unmakemove", move);
#endif

}

void makeCapture(unsigned int &captured, unsigned int &to)
{
	// deals with all captures, except en-passant
	BitMap toBitMap;
	toBitMap = BITSET[to];

	switch (captured)
	{
	case 1: // white pawn:
		board.whitePawns ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.Material -= PAWN_VALUE;
		break;

	case 2: // white king:
		board.whiteKing ^= toBitMap;
		board.whitePieces ^= toBitMap;
		break;

	case 3: // white knight:
		board.whiteKnights ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.Material -= KNIGHT_VALUE;
		break;

	case 5: // white bishop:
		board.whiteBishops ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.Material -= BISHOP_VALUE;
		break;

	case 6: // white rook:
		board.whiteRooks ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.Material -= ROOK_VALUE;
		if (to == A1) board.castleWhite &= ~CANCASTLEOOO;
		if (to == H1) board.castleWhite &= ~CANCASTLEOO;
		break;

	case 7: // white queen:
		board.whiteQueens ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.Material -= QUEEN_VALUE;
		break;

	case 9: // black pawn:
		board.blackPawns ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.Material += PAWN_VALUE;
		break;

	case 10: // black king:
		board.blackKing ^= toBitMap;
		board.blackPieces ^= toBitMap;
		break;

	case 11: // black knight:
		board.blackKnights ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.Material += KNIGHT_VALUE;
		break;

	case 13: // black bishop:
		board.blackBishops ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.Material += BISHOP_VALUE;
		break;

	case 14: // black rook:
		board.blackRooks ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.Material += ROOK_VALUE;
		if (to == A8) board.castleBlack &= ~CANCASTLEOOO;
		if (to == H8) board.castleBlack &= ~CANCASTLEOO;
		break;

	case 15: // black queen:
		board.blackQueens ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.Material += QUEEN_VALUE;
		break;
	}
	board.fiftyMove = 0;
}

void unmakeCapture(unsigned int &captured, unsigned int &to)
{
	// deals with all captures, except en-passant
	BitMap toBitMap;
	toBitMap = BITSET[to];

	switch (captured)
	{
	case 1: // white pawn:
		board.whitePawns ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.square[to] = WHITE_PAWN;
		board.Material += PAWN_VALUE;
		break;

	case 2: // white king:
		board.whiteKing ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.square[to] = WHITE_KING;
		break;

	case 3: // white knight:
		board.whiteKnights ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.square[to] = WHITE_KNIGHT;
		board.Material += KNIGHT_VALUE;
		break;

	case 5: // white bishop:
		board.whiteBishops ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.square[to] = WHITE_BISHOP;
		board.Material += BISHOP_VALUE;
		break;

	case 6: // white rook:
		board.whiteRooks ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.square[to] = WHITE_ROOK;
		board.Material += ROOK_VALUE;
		break;

	case 7: // white queen:
		board.whiteQueens ^= toBitMap;
		board.whitePieces ^= toBitMap;
		board.square[to] = WHITE_QUEEN;
		board.Material += QUEEN_VALUE;
		break;

	case 9: // black pawn:
		board.blackPawns ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.square[to] = BLACK_PAWN;
		board.Material -= PAWN_VALUE;
		break;

	case 10: // black king:
		board.blackKing ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.square[to] = BLACK_KING;
		break;

	case 11: // black knight:
		board.blackKnights ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.square[to] = BLACK_KNIGHT;
		board.Material -= KNIGHT_VALUE;
		break;

	case 13: // black bishop:
		board.blackBishops ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.square[to] = BLACK_BISHOP;
		board.Material -= BISHOP_VALUE;
		break;

	case 14: // black rook:
		board.blackRooks ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.square[to] = BLACK_ROOK;
		board.Material -= ROOK_VALUE;
		break;

	case 15: // black queen:
		board.blackQueens ^= toBitMap;
		board.blackPieces ^= toBitMap;
		board.square[to] = BLACK_QUEEN;
		board.Material -= QUEEN_VALUE;
		break;
	}
}

void makeWhitePromotion(unsigned int prom, unsigned int &to)
{
	BitMap toBitMap;
	toBitMap = BITSET[to];

	board.whitePawns ^= toBitMap;
	board.Material -= PAWN_VALUE;

	if (prom == 7)
	{
		board.whiteQueens ^= toBitMap;
		board.Material += QUEEN_VALUE;
	}
	else if (prom == 6)
	{
		board.whiteRooks ^= toBitMap;
		board.Material += ROOK_VALUE;
	}
	else if (prom == 5)
	{
		board.whiteBishops ^= toBitMap;
		board.Material += BISHOP_VALUE;
	}
	else if (prom == 3)
	{
		board.whiteKnights ^= toBitMap;
		board.Material += KNIGHT_VALUE;
	}
}

void unmakeWhitePromotion(unsigned int prom, unsigned int &to)
{
	BitMap toBitMap;
	toBitMap = BITSET[to];

	board.whitePawns ^= toBitMap;
	board.Material += PAWN_VALUE;

	if (prom == 7)
	{
		board.whiteQueens ^= toBitMap;
		board.Material -= QUEEN_VALUE;
	}
	else if (prom == 6)
	{
		board.whiteRooks ^= toBitMap;
		board.Material -= ROOK_VALUE;
	}
	else if (prom == 5)
	{
		board.whiteBishops ^= toBitMap;
		board.Material -= BISHOP_VALUE;
	}
	else if (prom == 3)
	{
		board.whiteKnights ^= toBitMap;
		board.Material -= KNIGHT_VALUE;
	}
}

void makeBlackPromotion(unsigned int prom, unsigned int &to)
{
	BitMap toBitMap;
	toBitMap = BITSET[to];

	board.blackPawns ^= toBitMap;
	board.Material += PAWN_VALUE;

	if (prom == 15)
	{
		board.blackQueens ^= toBitMap;
		board.Material -= QUEEN_VALUE;
	}
	else if (prom == 14)
	{
		board.blackRooks ^= toBitMap;
		board.Material -= ROOK_VALUE;
	}
	else if (prom == 13)
	{
		board.blackBishops ^= toBitMap;
		board.Material -= BISHOP_VALUE;
	}
	else if (prom == 11)
	{
		board.blackKnights ^= toBitMap;
		board.Material -= KNIGHT_VALUE;
	}
}

void unmakeBlackPromotion(unsigned int prom, unsigned int &to)
{
	BitMap toBitMap;
	toBitMap = BITSET[to];

	board.blackPawns ^= toBitMap;
	board.Material -= PAWN_VALUE;

	if (prom == 15)
	{
		board.blackQueens ^= toBitMap;
		board.Material += QUEEN_VALUE;
	}
	else if (prom == 14)
	{
		board.blackRooks ^= toBitMap;
		board.Material += ROOK_VALUE;
	}
	else if (prom == 13)
	{
		board.blackBishops ^= toBitMap;
		board.Material += BISHOP_VALUE;
	}
	else if (prom == 11)
	{
		board.blackKnights ^= toBitMap;
		board.Material += KNIGHT_VALUE;
	}
}

bool isOtherKingAttacked()
{
	// check to see if we are leaving our king in check
	if (board.nextMove)
	{
		return isAttacked(board.whiteKing, board.nextMove);
	}
	else
	{
		return isAttacked(board.blackKing, board.nextMove);
	}
}

bool isOwnKingAttacked()
{
	// check to see if we are leaving our king in check
	if (board.nextMove)
	{
		return isAttacked(board.blackKing, !board.nextMove);
	}
	else
	{
		return isAttacked(board.whiteKing, !board.nextMove);
	}
}

bool isValidTextMove(char *userMove, Move &move)
{
	// Checks if userMove is valid by comparing it with moves from the move generator
	// If found valid, the move is returned

	unsigned char userFrom, userTo, userPromote;
	BOOLTYPE moveFound;
	int i;

	if (strlen(userMove) > 3)
	{
		userFrom = userMove[0] - 97;
		userFrom += 8 * (userMove[1] - 49);
		userTo = userMove[2] - 97;
		userTo += 8 * (userMove[3] - 49);
	}

	userPromote = 0;
	if (strlen(userMove) > 4)
	{
		if (board.nextMove == WHITE_MOVE)
		{
			switch (userMove[4])
			{
			case 'q': userPromote = WHITE_QUEEN; break;
			case 'r': userPromote = WHITE_ROOK; break;
			case 'b': userPromote = WHITE_BISHOP; break;
			case 'n': userPromote = WHITE_KNIGHT; break;
			}
		}
		else
			switch (userMove[4])
			{
			case 'q': userPromote = BLACK_QUEEN; break;
			case 'r': userPromote = BLACK_ROOK; break;
			case 'b': userPromote = BLACK_BISHOP; break;
			case 'n': userPromote = BLACK_KNIGHT; break;
			}
	}

	moveFound = false;
	for (i = board.moveBufLen[0]; i < board.moveBufLen[1]; i++)
	{
		if ((board.moveBuffer[i].getFrom() == userFrom) && (board.moveBuffer[i].getTosq() == userTo))
		{
			if (((board.square[userFrom] == WHITE_PAWN) && (RANKS[userFrom] == 7)) ||
				((board.square[userFrom] == BLACK_PAWN) && (RANKS[userFrom] == 2)))
			{
				if (board.moveBuffer[i].getProm() == userPromote)
				{
					move.bitfield = board.moveBuffer[i].bitfield;
					return true;
				}
			}
			else
			{
				move.bitfield = board.moveBuffer[i].bitfield;
				return true;
			}
		}
	}
	move.bitfield = 0;
	return false;
}

#ifdef WINGLET_DEBUG_MOVES
void debugMoves(char *caller, Move &move)
{

	// check if board.square, piece bitmaps and whitepieces and blackpiese and occupied square are all consistent after (un)makmove

	char input[80];
	int mat, i, j;

	// check if both kings are present
	if ((bitCnt(board.whiteKing) != 1) || (bitCnt(board.blackKing) != 1))
	{
		std::cout << "king captured after  " << caller << std::endl;
		displayMove(move); std::cout << std::endl;
		for (j = 0; j < board.endOfSearch; j++)
		{
			std::cout << j + 1 << ". ";
			displayMove(board.gameLine[j].move);
			std::cout << std::endl;
		}
		board.display();
		std::cin >> input;
	}

	// check if board.square and piece bitmaps and whitepieces and blackpiese and occupied square are all telling the same
	for (i = 0; i < 64; i++)
	{
		if ((board.blackQueens & BITSET[i]) && (board.square[i] != BLACK_QUEEN))
		{
			std::cout << "inconsistency in black queens after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.blackRooks & BITSET[i]) && (board.square[i] != BLACK_ROOK))
		{
			std::cout << "inconsistency in black rooks after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			displayBitmap(board.blackRooks);
			std::cin >> input;
		}
		if ((board.blackBishops & BITSET[i]) && (board.square[i] != BLACK_BISHOP))
		{
			std::cout << "inconsistency in black bishops after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.blackKnights & BITSET[i]) && (board.square[i] != BLACK_KNIGHT))
		{
			std::cout << "inconsistency in black knights after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			displayMove(move);
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.blackKing & BITSET[i]) && (board.square[i] != BLACK_KING))
		{
			std::cout << "inconsistency in black king after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.blackPawns & BITSET[i]) && (board.square[i] != BLACK_PAWN))
		{
			std::cout << "inconsistency in black pawns after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			displayBitmap(board.blackPawns);
			std::cout.flush();
			std::cin >> input;
		}
		if ((board.whiteQueens & BITSET[i]) && (board.square[i] != WHITE_QUEEN))
		{
			std::cout << "inconsistency in white queens after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.whiteRooks & BITSET[i]) && (board.square[i] != WHITE_ROOK))
		{
			std::cout << "inconsistency in white rooks after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			displayBitmap(board.whiteRooks);
			std::cin >> input;
		}
		if ((board.whiteBishops & BITSET[i]) && (board.square[i] != WHITE_BISHOP))
		{
			std::cout << "inconsistency in white bishops after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.whiteKnights & BITSET[i]) && (board.square[i] != WHITE_KNIGHT))
		{
			std::cout << "inconsistency in white knights  after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.whiteKing & BITSET[i]) && (board.square[i] != WHITE_KING))
		{
			std::cout << "inconsistency in white king after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
		if ((board.whitePawns & BITSET[i]) && (board.square[i] != WHITE_PAWN))
		{
			std::cout << "inconsistency in white pawns after " << caller << std::endl;
			displayMove(move); std::cout << std::endl;
			for (j = 0; j < board.endOfSearch; j++)
			{
				std::cout << j + 1 << ". ";
				displayMove(board.gameLine[j].move);
				std::cout << std::endl;
			}
			board.display();
			std::cin >> input;
		}
	}

	if (board.whitePieces != (board.whiteKing | board.whiteQueens | board.whiteRooks | board.whiteBishops | board.whiteKnights | board.whitePawns))
	{
		std::cout << "inconsistency in whitePieces after " << caller << std::endl;
		displayMove(move); std::cout << std::endl;
		for (j = 0; j < board.endOfSearch; j++)
		{
			std::cout << j + 1 << ". ";
			displayMove(board.gameLine[j].move);
			std::cout << std::endl;
		}
		board.display();
		displayBitmap(board.whitePieces);
		std::cin >> input;
	}

	if (board.blackPieces != (board.blackKing | board.blackQueens | board.blackRooks | board.blackBishops | board.blackKnights | board.blackPawns))
	{
		std::cout << "inconsistency in blackPieces after " << caller << std::endl;
		displayMove(move); std::cout << std::endl;
		for (j = 0; j < board.endOfSearch; j++)
		{
			std::cout << j + 1 << ". ";
			displayMove(board.gameLine[j].move);
			std::cout << std::endl;
		}
		board.display();
		displayBitmap(board.blackPieces);
		std::cin >> input;
	}

	if (board.occupiedSquares != (board.whitePieces | board.blackPieces))
	{
		std::cout << "inconsistency in occupiedSquares after " << caller << std::endl;
		displayMove(move); std::cout << std::endl;
		for (j = 0; j < board.endOfSearch; j++)
		{
			std::cout << j + 1 << ". ";
			displayMove(board.gameLine[j].move);
			std::cout << std::endl;
		}
		board.display();
		displayBitmap(board.occupiedSquares);
		std::cin >> input;
	}

	mat = bitCnt(board.whitePawns) * PAWN_VALUE +
		bitCnt(board.whiteKnights) * KNIGHT_VALUE +
		bitCnt(board.whiteBishops) * BISHOP_VALUE +
		bitCnt(board.whiteRooks) * ROOK_VALUE +
		bitCnt(board.whiteQueens) * QUEEN_VALUE;
	mat -= (bitCnt(board.blackPawns) * PAWN_VALUE +
		bitCnt(board.blackKnights) * KNIGHT_VALUE +
		bitCnt(board.blackBishops) * BISHOP_VALUE +
		bitCnt(board.blackRooks) * ROOK_VALUE +
		bitCnt(board.blackQueens) * QUEEN_VALUE);
	if (board.Material != mat)
	{
		std::cout << "inconsistency in material after " << caller << std::endl;
		displayMove(move);
		board.display();
		std::cin >> input;
	}

}
#endif