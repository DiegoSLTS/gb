#pragma once

#include "IAddressable.h"

class MMU;

class Timer : public IAddressable {
public:
	MMU* mmu = nullptr;

	uint16_t dividerCounter = 0;
	uint16_t timerCounter = 0;

	uint8_t DIV = 0x00; //0xFF04 Divider Register (R/W)
	uint8_t TIMA = 0x00; //0xFF05 Timer counter (R/W)
	uint8_t TMA = 0x00; //0xFF06 Timer Modulo (R/W)
	uint8_t TAC = 0b11111000; //0xFF07 Timer Control (R/W)
	
	// TODO http://gbdev.gg8.se/wiki/articles/Timer_Obscure_Behaviour
	void Step(uint8_t cycles);

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;
};
