#include "SerialDataTransfer.h"

u8 SerialDataTransfer::Read(u16 address) {
	switch (address) {
	case 0xFF01:
		return SB;
	case 0xFF02:
		return SC;
	}
	return 0xFF;
}

void SerialDataTransfer::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFF01:
		SB = value; break;
	case 0xFF02:
		SC = value; break;
	}
}