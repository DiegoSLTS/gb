#pragma once

#include "IAddressable.h"
#include "IState.h"

class InterruptServiceRoutine : public IAddressable, public IState {
public:
	u8 IE = 0; // 0xFFFF - Interrupt Enabled (R/W)
	u8 IF = 0b11100000; // 0xFF0F - Interrupt Flag (R/W)

	bool IME = false; // not addressable - Interrupt Master Enable Flag(Write Only)
	bool eiDelay = false; // EI takes one more instruction to take effect

	bool IsInterruptEnabled(u8 interruptPosition);
	bool IsInterruptSet(u8 interruptPosition);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};