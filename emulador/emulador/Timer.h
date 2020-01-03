#pragma once

#include "IAddressable.h"

class MMU;

class Timer : public IAddressable {
public:
    Timer(MMU& mmu);
    virtual ~Timer();

	// TODO http://gbdev.gg8.se/wiki/articles/Timer_Obscure_Behaviour
	void Step(u8 cycles);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

private:
	MMU& mmu;

	u16 dividerCounter = 0;
	u16 timerCounter = 0;

	u8 DIV = 0x00;			//0xFF04 Divider Register (R/W)
	u8 TIMA = 0x00;			//0xFF05 Timer counter (R/W)
	u8 TMA = 0x00;			//0xFF06 Timer Modulo (R/W)
	u8 TAC = 0b11111000;	//0xFF07 Timer Control (R/W)
};
