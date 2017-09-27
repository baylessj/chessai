#pragma once
#include "bitops.h"
#include <string.h>

#define MAX_MOV_BUFF 4096    // Number of moves that we can store (all plies)
#define MAX_PLY      64      // Search depth

void movegenInit();

class Move {
public:
	Move() {
		this->clear();
	}
	void set(unsigned int piece, unsigned int captured, unsigned int from, unsigned int to, unsigned int promote);
	void setFrom(unsigned int from);
	void setTosq(unsigned int tosq);
	void setPiec(unsigned int piec);
	void setCapt(unsigned int capt);
	void setProm(unsigned int prom);
	void clear();

	unsigned int getFrom();
	unsigned int getTosq();
	unsigned int getPiec();
	unsigned int getCapt();
	unsigned int getProm();
	bool isWhitemove();
	bool isBlackmove();
	bool isCapture();
	bool isKingcaptured();
	bool isRookmove();
	bool isRookcaptured();
	bool isKingmove();
	bool isPawnmove();
	bool isPawnDoublemove();
	bool isEnpassant();
	bool isPromotion();
	bool isCastle();
	bool isCastleOO();
	bool isCastleOOO();

	unsigned int bitfield;
};

bool isAttacked(BitMap &targetBitmap, const unsigned char &fromSide);
int movegen(int index);

bool isValidTextMove(char *, Move &);
bool isOtherKingAttacked();
bool isOwnKingAttacked();

void makeBlackPromotion(unsigned int, unsigned int &);
void makeCapture(unsigned int &, unsigned int &);
void makeMove(Move &);
void makeWhitePromotion(unsigned int, unsigned int &);

void unmakeBlackPromotion(unsigned int, unsigned int &);
void unmakeCapture(unsigned int &, unsigned int &);
void unmakeMove(Move &);
void unmakeWhitePromotion(unsigned int, unsigned int &);

#define MAX_GAME_LINE 4096

struct GameLineRecord
{
	Move move;
	unsigned char castleWhite;     // White's castle status, CANCASTLEOO = 1, CANCASTLEOOO = 2
	unsigned char castleBlack;     // Black's castle status, CANCASTLEOO = 1, CANCASTLEOOO = 2
	int epSquare;                  // En-passant target square after double pawn move
	int fiftyMove;                 // Moves since the last pawn move or capture
};

extern BitMap WHITE_PAWN_ATTACKS[];
extern BitMap WHITE_PAWN_MOVES[];
extern BitMap WHITE_PAWN_DOUBLE_MOVES[];
extern BitMap BLACK_PAWN_ATTACKS[];
extern BitMap BLACK_PAWN_MOVES[];
extern BitMap BLACK_PAWN_DOUBLE_MOVES[];
extern BitMap KNIGHT_ATTACKS[];
extern BitMap KING_ATTACKS[];
extern BitMap RANK_ATTACKS[64][64];
extern BitMap FILE_ATTACKS[64][64];
extern BitMap DIAGA8H1_ATTACKS[64][64];
extern BitMap DIAGA1H8_ATTACKS[64][64];

extern const int RANKSHIFT[];
extern const BitMap _FILEMAGICS[];
extern const BitMap _DIAGA8H1MAGICS[];
extern const BitMap _DIAGA1H8MAGICS[];

extern BitMap RANKMASK[];
extern BitMap FILEMAGIC[];
extern BitMap FILEMASK[];
extern BitMap DIAGA8H1MASK[];
extern BitMap DIAGA8H1MAGIC[];
extern BitMap DIAGA1H8MASK[];
extern BitMap DIAGA1H8MAGIC[];

extern unsigned char GEN_SLIDING_ATTACKS[8][64];

extern BitMap maskEG[];
extern BitMap maskFG[];
extern BitMap maskBD[];
extern BitMap maskCE[];

extern int ICAPT;
extern int IEP;
extern int IPROM;
extern int ICASTLOO;
extern int ICASTLOOO;
extern int ICHECK;