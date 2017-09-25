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

										   // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										   // Black Pawns
										   // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(BLACK_PAWN);
		tempPiece = board.blackPawns;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = BLACK_PAWN_MOVES[from] & freeSquares;                // normal moves
			if (RANKS[from] == 7 && tempMove)
				tempMove |= (BLACK_PAWN_DOUBLE_MOVES[from] & freeSquares);  // add double moves
			tempMove |= BLACK_PAWN_ATTACKS[from] & board.whitePieces;       // add captures
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				if ((RANKS[to]) == 1)                                       // add promotions
				{
					move.setProm(BLACK_QUEEN);   board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(BLACK_ROOK);    board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(BLACK_BISHOP);  board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(BLACK_KNIGHT);  board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(EMPTY);
				}
				else
				{
					board.moveBuffer[index++].bitfield = move.bitfield;
				}
				tempMove ^= BITSET[to];
			}
			// add en-passant captures:
			if (board.epSquare)   // do a quick check first
			{
				if (BLACK_PAWN_ATTACKS[from] & BITSET[board.epSquare])
				{
					move.setProm(BLACK_PAWN);
					move.setCapt(WHITE_PAWN);
					move.setTosq(board.epSquare);
					board.moveBuffer[index++].bitfield = move.bitfield;
				}
			}
			tempPiece ^= BITSET[from];
			move.setProm(EMPTY);
		}

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

										   // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										   // White Pawns
										   // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		move.setPiec(WHITE_PAWN);
		tempPiece = board.whitePawns;
		while (tempPiece)
		{
			from = firstOne(tempPiece);
			move.setFrom(from);
			tempMove = WHITE_PAWN_MOVES[from] & freeSquares;                // normal moves
			if (RANKS[from] == 2 && tempMove)
				tempMove |= (WHITE_PAWN_DOUBLE_MOVES[from] & freeSquares);  // add double moves
			tempMove |= WHITE_PAWN_ATTACKS[from] & board.blackPieces;       // add captures
			while (tempMove)
			{
				to = firstOne(tempMove);
				move.setTosq(to);
				move.setCapt(board.square[to]);
				if ((RANKS[to]) == 8)                                       // add promotions
				{
					move.setProm(WHITE_QUEEN);   board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(WHITE_ROOK);    board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(WHITE_BISHOP);  board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(WHITE_KNIGHT);  board.moveBuffer[index++].bitfield = move.bitfield;
					move.setProm(EMPTY);
				}
				else
				{
					board.moveBuffer[index++].bitfield = move.bitfield;
				}
				tempMove ^= BITSET[to];
			}
			// add en-passant captures:
			if (board.epSquare)   // do a quick check first
			{
				if (WHITE_PAWN_ATTACKS[from] & BITSET[board.epSquare])
				{
					move.setProm(WHITE_PAWN);
					move.setCapt(BLACK_PAWN);
					move.setTosq(board.epSquare);
					board.moveBuffer[index++].bitfield = move.bitfield;
				}
			}
			tempPiece ^= BITSET[from];
			move.setProm(EMPTY);
		}

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