#pragma once

#include "IAddressable.h"
#include "IState.h"

class MMU;

class InterruptServiceRoutine : public IAddressable, public IState {
public:
	MMU* mmu = nullptr;

	uint8_t IE = 0; // 0xFFFF - Interrupt Enabled (R/W)
	uint8_t IF = 0b11100000; // 0xFF0F - Interrupt Flag (R/W)

	bool IME = false; // not addressable - Interrupt Master Enable Flag(Write Only)
	bool eiDelay = false; // EI takes one more instruction to take effect

	bool IsInterruptEnabled(uint8_t interruptPosition);
	bool IsInterruptSet(uint8_t interruptPosition);

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};