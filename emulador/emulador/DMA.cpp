#include "DMA.h"
#include "MMU.h"

void DMA::Step(u8 cycles) {
	while (currentCycles < 160 && cycles > 0) {
		mmu->Copy(addressBase + currentCycles, 0xFE00 + currentCycles);
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
