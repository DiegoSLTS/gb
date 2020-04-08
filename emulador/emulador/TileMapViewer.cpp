#include "TileMapViewer.h"
#include "GameBoy.h"
#include "MMU.h"
#include "GPU.h"
#include "Logger.h"

TileMapViewer::TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy, u8 mapNumber)
	: TileViewer(Width, Height, Title, Position, GameBoy), mapNumber(mapNumber), LCDC(gameBoy.gpu.GetLCDCRef()) {
    address = mapNumber == 0 ? 0x9800 : 0x9C00;
}

TileMapViewer::~TileMapViewer() {}

u16 TileMapViewer::GetTileAddress(u8 x, u8 y) const {
	u16 tileDataAddress = LCDC.bgWinData == 0 ? 0x8800 : 0x8000;

	u8 tileOffset = gameBoy.gpu.ReadVRAM(address + y * 32 + x, 0);
	return tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;
}

void TileMapViewer::UpdateTile(u8 x, u8 y) {
    u16 tileDataAddress = GetTileAddress(x, y);

	BGAttributes attributes = { isCGB ? gameBoy.gpu.ReadVRAM(address + y * 32 + x, 1) : (u8)0x00 };

    for (int line = 0; line < 8; line++) {
        u8 tileLine = attributes.flipY ? 7 - line : line;

		u8 lowByte = gameBoy.gpu.ReadVRAM(tileDataAddress + tileLine * 2, attributes.bank);
		u8 highByte = gameBoy.gpu.ReadVRAM(tileDataAddress + tileLine * 2 + 1, attributes.bank);

        u16 screenPosBase = (y * 8 + line) * screenTexture.getSize().x + x * 8;

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = attributes.flipX ? 7 - bit : bit;
            u16 screenPos = screenPosBase + (7 - bit);
            u8 lowBit = (lowByte >> pixel) & 0x01;
            u8 highBit = (highByte >> pixel) & 0x01;
			PixelInfo pixelInfo = { 0 };
			pixelInfo.colorIndex = lowBit | (highBit << 1);

            if (isCGB)
				pixelInfo.paletteIndex = attributes.paletteIndex;
			pixelInfo.isBG = true;
			((sf::Uint32*)screenArray)[screenPos] = gameBoy.gpu.GetABGR(pixelInfo).v;
        }
    }
}

void TileMapViewer::PrintTile(u8 x, u8 y) {
    u16 tileDataAddress = GetTileAddress(x, y);
    BGAttributes attributes = { isCGB ? gameBoy.gpu.ReadVRAM(address + y * 32 + x, 1) : (u8)0x00 };
    printf("\nTile X: %d Y: %d\n", x, y);
    printf("Address: %s\n", Logger::u16ToHex(tileDataAddress).c_str());
    printf("Bank: %d\n", attributes.bank);
    printf("Flip X: %d\n", attributes.flipX);
    printf("Flip Y: %d\n", attributes.flipY);
    printf("Priority: %d\n", attributes.hasPriority);
    printf("Palette: %d\n", attributes.paletteIndex);
}

void TileMapViewer::OnMouseClicked(u32 x, u32 y) {
    u8 newX = x / 16; // tile size (8) * window scale (2)
    u8 newY = y / 16;

    if (newX != loggedTileX || newY != loggedTileY) {
        loggedTileX = newX;
        loggedTileY = newY;
        PrintTile(loggedTileX, loggedTileY);
    }
}
