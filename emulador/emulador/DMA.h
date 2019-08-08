#pragma once

#include "IAddressable.h"
// DMA: https://gekkio.fi/files/gb-docs/gbctr.pdf

class MMU;

//TODO IState
class DMA : public IAddressable {
public:
	MMU* mmu = nullptr;

	uint8_t currentCycles = 160;
	uint16_t addressBase = 0;

	void Step(uint8_t cycles);

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;
};