#pragma once

#include "bitops.h"

extern const unsigned char WHITE_MOVE;
extern const unsigned char BLACK_MOVE;

extern const unsigned char EMPTY;
extern const unsigned char WHITE_KNIGHT;
extern const unsigned char WHITE_PAWN;
extern const unsigned char WHITE_KING;
extern const unsigned char WHITE_BISHOP;
extern const unsigned char WHITE_ROOK;
extern const unsigned char WHITE_QUEEN;
extern const unsigned char BLACK_KNIGHT;
extern const unsigned char BLACK_PAWN;
extern const unsigned char BLACK_KING;
extern const unsigned char BLACK_BISHOP;
extern const unsigned char BLACK_ROOK;
extern const unsigned char BLACK_QUEEN;

extern const char* PIECENAMES[];

extern BitMap BITSET[];
extern int BOARDINDEX[9][9];

extern const int PAWN_VALUE;
extern const int KNIGHT_VALUE;
extern const int BISHOP_VALUE;
extern const int ROOK_VALUE;
extern const int QUEEN_VALUE;
extern const int KING_VALUE;
extern const int CHECK_MATE;

extern unsigned char CANCASTLEOO;
extern unsigned char CANCASTLEOOO;
extern unsigned int WHITE_OOO_CASTL;
extern unsigned int BLACK_OOO_CASTL;
extern unsigned int WHITE_OO_CASTL;
extern unsigned int BLACK_OO_CASTL;
