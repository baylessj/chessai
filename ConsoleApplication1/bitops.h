#pragma once

typedef unsigned long long BitMap;

extern int MS1BTABLE[];

unsigned int bitCnt(BitMap);
unsigned int lastOne(BitMap);
void displayBitmap(BitMap);
unsigned int firstOne(BitMap);