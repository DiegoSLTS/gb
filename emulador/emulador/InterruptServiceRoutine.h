#pragma once

#include "IAddressable.h"
#include "IState.h"

enum class InterruptFlag : u8 {
	VBlank = 0x01,
	LCDStat = 0x02,
	Timer = 0x04,
	Serial = 0x08,
	Joypad = 0x10
};

class InterruptServiceRoutine : public IAddressable, public IState {
public:
	InterruptServiceRoutine();
	virtual ~InterruptServiceRoutine();

	u8 IE = 0;		// 0xFFFF - Interrupt Enabled (R/W)
	u8 IF = 0xE0;	// 0xFF0F - Interrupt Flag (R/W)

	bool IME = false; // not addressable - Interrupt Master Enable Flag(Write Only)
	bool eiDelay = false; // EI takes one more instruction to take effect

	bool IsInterruptEnabled(InterruptFlag flag);
	bool IsInterruptSet(InterruptFlag flag);
	void SetInterruptFlag(InterruptFlag flag);
	void ResetInterruptFlag(InterruptFlag flag);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
    
    bool log = false;
};
