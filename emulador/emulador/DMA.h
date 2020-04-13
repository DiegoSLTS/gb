#pragma once

#include "IAddressable.h"
#include "IState.h"

class MMU;
class Logger;

class DMA : public IAddressable, public IState {
public:
    DMA(MMU& mmu, u8* oam);
    virtual ~DMA();

	void Step(u8 cycles);
    void StepHDMA();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;

    bool log = false;
    
private:
    MMU& mmu;

    // DMA
	u8* oam = nullptr; // cached pointer to OAM to avoid going through MMU for each write
    u8 currentCycles = 160;
    u16 addressBase = 0;

    // HDMA
    u16 HDMASource = 0; // msb = HDMA1 (0xFF51), lsb = HDMA2 (0xFF52)
    u16 HDMADest = 0x8000; // msb = HDMA3 (0xFF53), lsb = HDMA4 (0xFF54)
    
    bool hdmaInProgress = false;
    u8 remaining = 0x7F;

    void DoHDMATransfer(u16 count);
};
