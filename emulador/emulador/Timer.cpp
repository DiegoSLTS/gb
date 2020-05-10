#include "Timer.h"
#include "InterruptServiceRoutine.h"

Timer::Timer(InterruptServiceRoutine& interruptService) : interruptService(interruptService) {}
Timer::~Timer() {}

void Timer::Step(u8 cycles) {
    bool wasHigh = TACInput == 1;
    timerReg += cycles;
    UpdateTACInput();
    UpdateTIMA(wasHigh);
}

u8 Timer::Read(u16 address) {
	switch (address) {
	case 0xFF04:
        return (u8)(timerReg >> 8);
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
    case 0xFF04: {
        bool wasHigh = TACInput == 1;
        timerReg = 0;
        UpdateTACInput();
        
    }
    break;
	case 0xFF05:
		TIMA = value; break;
	case 0xFF06:
		TMA = value; break;
    case 0xFF07: {
        bool wasEnabled = TAC & 0x04;
        bool wasHigh = TACInput == 1;
        TAC = value | 0xF8;
        UpdateTACInput();
        if (wasEnabled)
            UpdateTIMA(wasHigh);
    }
    break;
	}
}

void Timer::UpdateTACInput() {
    u8 timeFreq = TIMABits[TAC & 0x03];
    TACInput = (timerReg >> timeFreq) & 0x01;
}

void Timer::UpdateTIMA(u8 wasHigh) {
    if (TAC & 0x04) { // is on
        if (wasHigh && TACInput == 0) {
            TIMA++;
            if (TIMA == 0) {
                TIMA = TMA;
                interruptService.SetInterruptFlag(InterruptFlag::Timer);
            }
        }
    }
}
