#pragma once
/*
76543210
ZNHC0000

Zero Flag (Z):This bit is set when the result of a math operationis zero or two values match when using the CPinstruction.
Subtract Flag (N):This bit is set if a subtraction was performed in thelast math instruction.
Half Carry Flag (H):This bit is set if a carry occurred from the lowernibble in the last math operation.
Carry Flag (C):This bit is set if a carry occurred from the lastmath operation or if register A is the smaller valuewhen executing the CP instruction.
*/

#include <cstdint>

enum FlagBit : u8 {
	None		= 0,
	Bit0		= 1 << 0,
	Bit1		= 1 << 1,
	Bit2		= 1 << 2,
	Bit3		= 1 << 3,
	Carry		= 1 << 4,
	HalfCarry	= 1 << 5,
	Negative	= 1 << 6,
	Zero		= 1 << 7
};