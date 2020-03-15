#include "TileMapViewer.h"
#include "GameBoy.h"
#include "MMU.h"
#include "GPU.h"

TileMapViewer::TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy, u8 mapNumber) : TileViewer(Width, Height, Title, Position, GameBoy), mapNumber(mapNumber) {
    address = mapNumber == 0 ? 0x9800 : 0x9C00;
}

TileMapViewer::~TileMapViewer() {}

u16 TileMapViewer::GetTileAddress(u8 x, u8 y) const {
	u8 LCDCRegister = gameBoy.mmu.Read(0xFF40);
	u16 tileDataAddress = (LCDCRegister & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	u8 tileOffset = gameBoy.gpu.ReadVRAM0(address + y * 32 + x);
	return tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;
}

void TileMapViewer::UpdateTile(u8 x, u8 y) {
    u16 tileDataAddress = GetTileAddress(x, y);

    u8 cgbTileAttributes = isCGB ? gameBoy.gpu.ReadVRAM1(address + y * 32 + x) : 0x00;

    u8 cgbPaletteIndex = cgbTileAttributes & 0x07;
    u8 VRAMBank = ((cgbTileAttributes >> 3) & 0x01);
    bool flipX = ((cgbTileAttributes & 0x20) != 0);
    bool flipY = ((cgbTileAttributes & 0x40) != 0);

    for (int line = 0; line < 8; line++) {
        u8 lowByte = 0;
        u8 highByte = 0;

        u8 tileLine = flipY ? 7 - line : line;

        if (VRAMBank == 0) {
            lowByte = gameBoy.gpu.ReadVRAM0(tileDataAddress + tileLine * 2);
            highByte = gameBoy.gpu.ReadVRAM0(tileDataAddress + tileLine * 2 + 1);
        }
        else {
            lowByte = gameBoy.gpu.ReadVRAM1(tileDataAddress + tileLine * 2);
            highByte = gameBoy.gpu.ReadVRAM1(tileDataAddress + tileLine * 2 + 1);
        }

        u16 screenPosBase = (y * 8 + line) * screenTexture.getSize().x + x * 8;

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = flipX ? 7 - bit : bit;
            u16 screenPos = screenPosBase + (7 - bit);
            u8 lowBit = (lowByte >> pixel) & 0x01;
            u8 highBit = (highByte >> pixel) & 0x01;
            u8 index = lowBit | (highBit << 1);

            if (isCGB)
				index |= (cgbPaletteIndex << 2);
			index |= 0x20;
			((sf::Uint32*)screenArray)[screenPos] = gameBoy.gpu.GetABGR(index);
        }
    }
}
