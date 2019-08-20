#include "DMA.h"
#include "MMU.h"

DMA::DMA(MMU& mmu) : mmu(mmu) {}
DMA::~DMA() {}

void DMA::Step(u8 cycles) {
	while (currentCycles < 160 && cycles > 0) {
		mmu.Copy(addressBase + currentCycles, 0xFE00 + currentCycles);
		currentCycles++;
		cycles--;
	}
}

u8 DMA::Read(u16 address) {
	//TODO confirm this?
	return addressBase >> 8;
}

void DMA::Write(u8 value, u16 address) {
	//TODO assert address is valid
	currentCycles = 0;
	addressBase = value << 8;
}

void DMA::Load(std::ifstream& stream) const {
    stream.read((char*)&currentCycles, 3);
}

void DMA::Save(std::ofstream& stream) const {
    stream.write((char*)&currentCycles, 3);
}
