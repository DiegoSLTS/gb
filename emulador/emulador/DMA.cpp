#include "DMA.h"
#include "MMU.h"

void DMA::Step(uint8_t cycles) {
	while (currentCycles < 160 && cycles > 0) {
		mmu->Copy(addressBase + currentCycles, 0xFE00 + currentCycles);
		currentCycles++;
		cycles--;
	}
}

uint8_t DMA::Read(uint16_t address) {
	//TODO confirm this?
	return addressBase >> 8;
}

void DMA::Write(uint8_t value, uint16_t address) {
	//TODO assert address is valid
	currentCycles = 0;
	addressBase = value << 8;
}
