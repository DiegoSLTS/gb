#pragma once

#include "IAddressable.h"
#include "IState.h"
// DMA: https://gekkio.fi/files/gb-docs/gbctr.pdf

class MMU;

class DMA : public IAddressable, public IState {
public:
    DMA(MMU& mmu);
    virtual ~DMA();

    MMU& mmu;

    u8 currentCycles = 160;
    u16 addressBase = 0;

	void Step(u8 cycles);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

    virtual void Load(std::ifstream& stream) const override;
    virtual void Save(std::ofstream& stream) const override;
};