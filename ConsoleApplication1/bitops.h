#pragma once

typedef unsigned long long BitMap;

extern int MS1BTABLE[];
extern BitMap BITSET[];

void initBitOps();
unsigned int bitCnt(BitMap);
unsigned int ms1b(BitMap);
unsigned int ls1b(BitMap);
void displayBitmap(BitMap);