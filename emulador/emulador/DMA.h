#pragma once

#include "IAddressable.h"
// DMA: https://gekkio.fi/files/gb-docs/gbctr.pdf

class MMU;

//TODO IState
class DMA : public IAddressable {
public:
	MMU* mmu = nullptr;

    u8 currentCycles = 160;
    u16 addressBase = 0;

	void Step(u8 cycles);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;
};