#include "Timer.h"
#include "MMU.h"

void Timer::Step(uint8_t cycles) {
	dividerCounter += cycles;
	if (dividerCounter >= 255) {
		DIV++;
		dividerCounter -= 255;
	}
	
	if (TAC & 0x04) { // is on
		timerCounter += cycles;

		uint8_t frequency = TAC & 0x03;

		uint16_t maxCounter = 0;

		switch (frequency) {
		case 0: maxCounter = 1024; break; // freq 4096
		case 1: maxCounter = 16; break;// freq 262144
		case 2: maxCounter = 64; break;// freq 65536
		case 3: maxCounter = 256; break;// freq 16382
		}

		if (timerCounter >= maxCounter) {
			TIMA++;
			if (TIMA == 0) {
				TIMA = TMA;
				mmu->SetInterruptFlag(2);
			}
			timerCounter -= maxCounter;
		}
	}
}

uint8_t Timer::Read(uint16_t address) {
	switch (address) {
	case 0xFF04:
		return DIV;
	case 0xFF05:
		return TIMA;
	case 0xFF06:
		return TMA;
	case 0xFF07:
		return TAC | 0b11111000;
	}
	return 0xFF;
}

void Timer::Write(uint8_t value, uint16_t address) {
	switch (address) {
	case 0xFF04:
		DIV = 0; TIMA = 0; break;
	case 0xFF05:
		TIMA = value; break;
	case 0xFF06:
		TMA = value; break;
	case 0xFF07:
		TAC = value | 0b11111000; break;
	}
}