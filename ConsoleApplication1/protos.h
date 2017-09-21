#pragma once

#include "board.h"

unsigned int  bitCnt(BitMap);
void          dataInit();
void          displayBitmap(BitMap);
bool          doCommand(const char *);
unsigned int  firstOne(BitMap);
void          info();
unsigned int  lastOne(BitMap);
void          readCommands();
bool          readfen();
bool          readFen(char *, int);
void          setupFen(char *, char *, char *, char *, int, int);
void          setup();