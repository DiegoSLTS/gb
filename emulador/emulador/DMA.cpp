#include "DMA.h"
#include "MMU.h"

DMA::DMA(MMU& mmu, u8* oam) : mmu(mmu), oam(oam) {}
DMA::~DMA() {}

void DMA::Step(u8 cycles) {
	while (currentCycles < 160 && cycles > 0) {
		oam[currentCycles] = mmu.Read(addressBase + currentCycles);
		currentCycles++;
		cycles--;
	}
}

void DMA::StepHDMA() {
    if (!hdmaInProgress)
        return;

    u16 length = 0x10;
    u16 source = (HDMA1 << 8) | (HDMA2 & 0xF0) + (hdmaProgress * 0x10);
    u16 dest = 0x8000 + ((HDMA3 & 0x1F) << 8) | (HDMA4 & 0xF0) + (hdmaProgress * 0x10);
    for (u16 i = 0; i < length; i++)
        mmu.Copy(source + i, dest + i);

    if (remaining > 0) {
        remaining--;
        hdmaProgress++;
    } else {
        remaining = 0x7F;
        hdmaInProgress = false;
    }
    
    // TODO emulate time spent
}

u8 DMA::Read(u16 address) {
	switch (address) {
	case 0xFF46:
		//TODO confirm this?
		return addressBase >> 8;
	case 0xFF51:
		return HDMA1;
	case 0xFF52:
		return HDMA2;
	case 0xFF53:
		return HDMA3;
	case 0xFF54:
		return HDMA4;
	case 0xFF55:
    {
        u8 bit7 = hdmaInProgress ? 0 : 0x80;
        return bit7 | remaining;
    }
	}

	return 0xFF;
}

void DMA::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFF46:
		//TODO assert addressBase is valid?
		currentCycles = 0;
		addressBase = value << 8;
		break;
	case 0xFF51:
		HDMA1 = value; break;
	case 0xFF52:
		HDMA2 = value; break;
	case 0xFF53:
		HDMA3 = value; break;
	case 0xFF54:
		HDMA4 = value; break;
	case 0xFF55:
        if (hdmaInProgress) {
            if ((value & 0x80) == 0) {
                hdmaInProgress = false;
            }
        } else {
            if ((value & 0x80) == 0) {
                u16 length = ((value & 0x7F) + 1) * 0x10;
                u16 source = (HDMA1 << 8) | (HDMA2 & 0xF0);
                u16 dest = 0x8000 + ((HDMA3 & 0x1F) << 8) | (HDMA4 & 0xF0);
                for (u16 i = 0; i < length; i++)
                    mmu.Copy(source + i, dest + i);
                remaining = 0x7F;
                hdmaInProgress = false;
                // TODO emulate time spent
            } else {
                hdmaInProgress = true;
                remaining = value & 0x7F;
                hdmaProgress = 0;
            }
        }

        break;
	}
	
}

void DMA::Load(std::ifstream& stream) const {
    stream.read((char*)&currentCycles, 3);
}

void DMA::Save(std::ofstream& stream) const {
    stream.write((char*)&currentCycles, 3);
}
