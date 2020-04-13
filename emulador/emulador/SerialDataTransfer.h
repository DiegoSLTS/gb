#pragma once

#include "IAddressable.h"

class SerialDataTransfer : public IAddressable {
public:

	SerialDataTransfer();
	virtual ~SerialDataTransfer();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

    void Step(u8 cycles, bool isDoubleSpeedEnabled);

private:
	u8 SB = 0;		//0xFF01 Serial Transfer data (R/W)
	u8 SC = 0x7E;	//0xFF02 Serial Transfer Control (R/W)
    bool normalClock = true;

    u16 elapsedCycles = 0;
    u16 maxCycles = 0;
    u8 bitsTransfered = 8;
};
