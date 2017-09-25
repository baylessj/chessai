#pragma once

class Move {
public:
	void set(unsigned int piece, unsigned int captured, unsigned int from, unsigned int to, unsigned int promote);

private:
	unsigned int bitfield;
};