#include "SerialDataTransfer.h"

SerialDataTransfer::SerialDataTransfer() {}
SerialDataTransfer::~SerialDataTransfer() {}

u8 SerialDataTransfer::Read(u16 address) {
	switch (address) {
	case 0xFF01:
		return SB;
	case 0xFF02:
		return SC | 0x7E;
	}
	return 0xFF;
}

void SerialDataTransfer::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFF01:
		SB = value; break;
	case 0xFF02:
		SC = value | 0x7E; break;
	}
}
