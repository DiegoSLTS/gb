#include "SpritesViewer.h"
#include "GameBoy.h"
#include "MMU.h"
#include "GPU.h"

SpritesViewer::SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy) : Window(Width, Height, Title, Position, false), gameBoy(GameBoy) {
	Update();

    isCGB = gameBoy.IsCGB;
}

SpritesViewer::~SpritesViewer() {}

void SpritesViewer::Update() {
    if (!IsOpen())
        return;

	memset(screenArray, 0, width * height * 4);
	for (u8 i = 0; i < 40; i++)
		DrawSprite(i);

	screenTexture.update(screenArray);
	screenSprite.setTexture(screenTexture, true);

	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

void SpritesViewer::DrawSprite(u8 index) {
	const u8 paletteMask = 0b00000011;

	u8 LCDC = gameBoy.mmu.Read(0xFF40);
	u8 spriteHeight = LCDC & LCDCMask::ObjSize ? 16 : 8;
	u8 spriteIndex = index * 4;

	u8 spriteY = gameBoy.mmu.Read(0xFE00 + spriteIndex);
	u8 spriteX = gameBoy.mmu.Read(0xFE00 + spriteIndex + 1);
	u8 tileIndex = gameBoy.mmu.Read(0xFE00 + spriteIndex + 2);
	u8 attributes = gameBoy.mmu.Read(0xFE00 + spriteIndex + 3);

	bool hasPriority = (attributes & 0b10000000) > 0;
	bool flipY = (attributes & 0b01000000) > 0;
	bool flipX = (attributes & 0b00100000) > 0;
	u8 gbPalette =(attributes & 0b00010000) > 0 ? 1 : 0; //0xFF49 OBP1, 0xFF48 OBP0
    u8 VRAMBank = (attributes & 0b00001000) > 0 ? 1 : 0;
    u8 cgbPaletteIndex = attributes & 0x7;

	u16 tileAddress = 0x8000 + tileIndex * 16;

	for (u8 line = 0; line < spriteHeight; line++) {
        u8 spriteLine = flipY ? spriteHeight - 1 - line : line;
		u8 lowByte = 0;
		u8 highByte = 0;

        if (VRAMBank == 0) {
            lowByte = gameBoy.gpu.ReadVRAM0(tileAddress + spriteLine * 2);
            highByte = gameBoy.gpu.ReadVRAM0(tileAddress + spriteLine * 2 + 1);
        }
        else {
            lowByte = gameBoy.gpu.ReadVRAM1(tileAddress + spriteLine * 2);
            highByte = gameBoy.gpu.ReadVRAM0(tileAddress + spriteLine * 2 + 1);
        }

        u16 screenPosBase = (spriteY + line - 1) * (256 + 8) + spriteX;

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = flipX ? 7 - bit : bit;
            u16 screenPos = screenPosBase + (7 - bit);
            
			u8 lowBit = (lowByte >> pixel) & 0x01;
			u8 highBit = (highByte >> pixel) & 0x01;
			u8 index = lowBit | (highBit << 1);

            if (index > 0) {
				u8 pixelInfo = index;
				if (isCGB)
					pixelInfo |= (cgbPaletteIndex << 2);
				else
					pixelInfo |= (gbPalette << 2);

				((sf::Uint32*)screenArray)[screenPos] = gameBoy.gpu.GetABGR(pixelInfo);
            }
        }
	}
}
