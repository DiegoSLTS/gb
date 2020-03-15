#pragma once

#include "IAddressable.h"
#include "IState.h"
// DMA: https://gekkio.fi/files/gb-docs/gbctr.pdf

class MMU;

class DMA : public IAddressable, public IState {
public:
    DMA(MMU& mmu);
    virtual ~DMA();

	void Step(u8 cycles);
    void StepHDMA();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;

private:
    MMU& mmu;

    u8 currentCycles = 160;
    u16 addressBase = 0;

	u8 HDMA1 = 0; // 0xFF51
	u8 HDMA2 = 0; // 0xFF52
	u8 HDMA3 = 0; // 0xFF53
	u8 HDMA4 = 0; // 0xFF54
	u8 HDMA5 = 0; // 0xFF55

	u16 hdmaSource = 0;
	u16 hdmaDestination = 0;

    bool hdmaInProgress = false;
    u8 remaining = 0x7F;
    u8 hdmaProgress = 0;
};
