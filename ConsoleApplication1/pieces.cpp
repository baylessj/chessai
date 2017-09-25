#include "pieces.h"
#include "board.h"

// Identifier of next move:
extern const unsigned char WHITE_MOVE = 0;
extern const unsigned char BLACK_MOVE = 1;

// Piece identifiers, 4 bits each.
// Usefull bitwise properties of this numbering scheme:
// white = 0..., black = 1..., sliding = .1.., nonsliding = .0..
// rank/file sliding pieces = .11., diagonally sliding pieces = .1.1
// pawns and kings (without color bits), are < 3
// major pieces (without color bits set), are > 5
// minor and major pieces (without color bits set), are > 2
extern const unsigned char EMPTY = 0;                //  0000
extern const unsigned char WHITE_PAWN = 1;           //  0001
extern const unsigned char WHITE_KING = 2;           //  0010
extern const unsigned char WHITE_KNIGHT = 3;         //  0011
extern const unsigned char WHITE_BISHOP = 5;        //  0101
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

BitMap BITSET[64];
int BOARDINDEX[9][9]; // index 0 is not used, only 1..8.

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

void Knight::generateMoves() {
	BitMap target = ~board.whitePieces; // possible moves are any squares besides those occupied by the white pieces
	BitMap tempPiece = board.whiteKnights;

	while (tempPiece) {
		unsigned int from = firstOne(tempPiece);
		unsigned int tempMove = attacks[from] & target;
	}
}