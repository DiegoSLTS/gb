#include "Timer.h"
#include "InterruptServiceRoutine.h"

Timer::Timer(InterruptServiceRoutine& interruptService) : interruptService(interruptService) {}
Timer::~Timer() {}

void Timer::Step(u8 cycles) {
	dividerCounter += cycles;
	if (dividerCounter >= 255) {
		DIV++;
		dividerCounter -= 255;
	}

    timerCounter += cycles;

	if (TAC & 0x04) { // is on
		u8 frequency = TAC & 0x03;
		u16 maxCounter = 0;

		switch (frequency) {
		case 0: maxCounter = 1024; break;	// freq 4096
		case 1: maxCounter = 16; break;		// freq 262144
		case 2: maxCounter = 64; break;		// freq 65536
		case 3: maxCounter = 256; break;	// freq 16382
		}

		if (timerCounter >= maxCounter) {
			TIMA++;
			if (TIMA == 0) {
				TIMA = TMA;
				interruptService.SetInterruptFlag(InterruptFlag::Timer);
			}
			timerCounter -= maxCounter;
		}
	}
}

u8 Timer::Read(u16 address) {
	switch (address) {
	case 0xFF04:
		return DIV;
	case 0xFF05:
		return TIMA;
	case 0xFF06:
		return TMA;
	case 0xFF07:
		return TAC | 0xF8;
	}
	return 0xFF;
}

void Timer::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFF04:
        DIV = 0; TIMA = 0; timerCounter = 0; break;
	case 0xFF05:
		TIMA = value; break;
	case 0xFF06:
		TMA = value; break;
	case 0xFF07:
		TAC = value | 0xF8; break;
	}
}
