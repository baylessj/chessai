#include "movement.h"

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