#pragma once

#include "bitops.h"

extern int PENALTY_DOUBLED_PAWN;
extern int PENALTY_ISOLATED_PAWN;
extern int PENALTY_BACKWARD_PAWN;
extern int BONUS_PASSED_PAWN;
extern int BONUS_BISHOP_PAIR;
extern int BONUS_ROOK_BEHIND_PASSED_PAWN;
extern int BONUS_ROOK_ON_OPEN_FILE;
extern int BONUS_PAWN_SHIELD_STRONG;
extern int BONUS_PAWN_SHIELD_WEAK;
extern int PAWN_OWN_DISTANCE[];
extern int PAWN_OPPONENT_DISTANCE[];
extern int KNIGHT_DISTANCE[];
extern int BISHOP_DISTANCE[];
extern int ROOK_DISTANCE[];
extern int QUEEN_DISTANCE[];
extern int PAWNPOS_W[];
extern int KNIGHTPOS_W[];
extern int BISHOPPOS_W[];
extern int ROOKPOS_W[];
extern int QUEENPOS_W[];
extern int KINGPOS_W[];
extern int KINGPOS_ENDGAME_W[];
extern int MIRROR[];
extern int DISTANCE[64][64];
extern int PAWNPOS_B[];
extern int KNIGHTPOS_B[];
extern int BISHOPPOS_B[];
extern int ROOKPOS_B[];
extern int QUEENPOS_B[];
extern int KINGPOS_B[];
extern int KINGPOS_ENDGAME_B[];
extern BitMap PASSED_WHITE[];
extern BitMap PASSED_BLACK[];
extern BitMap ISOLATED_WHITE[];
extern BitMap ISOLATED_BLACK[];
extern BitMap BACKWARD_WHITE[];
extern BitMap BACKWARD_BLACK[];
extern BitMap KINGSHIELD_STRONG_W[];
extern BitMap KINGSHIELD_STRONG_B[];
extern BitMap KINGSHIELD_WEAK_W[];
extern BitMap KINGSHIELD_WEAK_B[];
extern BitMap WHITE_SQUARES;
extern BitMap BLACK_SQUARES;