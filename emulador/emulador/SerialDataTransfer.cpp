#include "SerialDataTransfer.h"

uint8_t SerialDataTransfer::Read(uint16_t address) {
	switch (address) {
	case 0xFF01:
		return SB;
	case 0xFF02:
		return SC;
	}
	return 0xFF;
}

void SerialDataTransfer::Write(uint8_t value, uint16_t address) {
	switch (address) {
	case 0xFF01:
		SB = value; break;
	case 0xFF02:
		SC = value; break;
	}
}