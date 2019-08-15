#include "TileMapViewer.h"
#include "MMU.h"
#include "GPU.h"

TileMapViewer::TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, u16 Address) : TileViewer(Width, Height, Title, Mmu, Address) {}
TileMapViewer::~TileMapViewer() {}

u16 TileMapViewer::GetTileAddress(u8 x, u8 y) const {
	u8 LCDCRegister = mmu->Read(0xFF40);
	u16 tileDataAddress = (LCDCRegister & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	u8 tileOffset = mmu->Read(address + y * 32 + x);
	return tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;
}
