#include "TileMapViewer.h"
#include "MMU.h"
#include "GPU.h"

TileMapViewer::TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, uint16_t Address) : TileViewer(Width, Height, Title, Mmu, Address) {}
TileMapViewer::~TileMapViewer() {}

uint16_t TileMapViewer::GetTileAddress(uint8_t x, uint8_t y) const {
	uint8_t LCDCRegister = mmu->Read(0xFF40);
	uint16_t tileDataAddress = (LCDCRegister & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	uint8_t tileOffset = mmu->Read(address + y * 32 + x);
	return tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((int8_t)tileOffset) * 16; //TODO revisar
}
