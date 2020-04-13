#include "SerialDataTransfer.h"

SerialDataTransfer::SerialDataTransfer() {}
SerialDataTransfer::~SerialDataTransfer() {}

u8 SerialDataTransfer::Read(u16 address) {
	switch (address) {
	case 0xFF01:
		return SB;
	case 0xFF02:
		return SC | 0x7C;
	}
	return 0xFF;
}

void SerialDataTransfer::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFF01:
		SB = value; break;
	case 0xFF02:
		SC = value | 0x7C;
        bitsTransfered = 0;
        elapsedCycles = 0;
        normalClock = ((value & 0x02) == 0);
        maxCycles = normalClock ? 512 : 16; // 8KHz or 32KHz
        break;
	}
}

void SerialDataTransfer::Step(u8 cycles, bool isDoubleSpeedEnabled) {
    if (bitsTransfered == 8)
        return;

    elapsedCycles += (isDoubleSpeedEnabled ? cycles * 2 : cycles);
    while (elapsedCycles >= maxCycles) {
        bitsTransfered++;
        elapsedCycles -= maxCycles;

        if (bitsTransfered == 8) {
            SC &= ~0x80;
            return;
        }
    }
}
