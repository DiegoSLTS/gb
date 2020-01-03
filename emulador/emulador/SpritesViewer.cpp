#include "SpritesViewer.h"
#include "MMU.h"
#include "GPU.h"

SpritesViewer::SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU& Mmu) : Window(Width, Height, Title), mmu(Mmu) {
	Update();
}

SpritesViewer::~SpritesViewer() {}

void SpritesViewer::Update() {
	memset(screenArray.get(), 0, (256 + 8) * (256 + 16) * 4);
	for (u8 i = 0; i < 40; i++)
		DrawSprite(i);

	screenTexture.update(screenArray.get());
	screenSprite.setTexture(screenTexture, true);

	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

void SpritesViewer::DrawSprite(u8 index) {
	const u8 paletteMask = 0b00000011;

	u8 LCDC = mmu.Read(0xFF40);
	u8 spriteHeight = LCDC & LCDCMask::ObjSize ? 16 : 8;
	u8 spriteIndex = index * 4;

	u8 spriteY = mmu.Read(0xFE00 + spriteIndex);
	u8 spriteX = mmu.Read(0xFE00 + spriteIndex + 1);
	u8 tileIndex = mmu.Read(0xFE00 + spriteIndex + 2);
	u8 attributes = mmu.Read(0xFE00 + spriteIndex + 3);

	bool hasPriority = (attributes & 0b10000000) > 0;
	bool flipY = (attributes & 0b01000000) > 0;
	bool flipX = (attributes & 0b00100000) > 0;
	u16 palette = mmu.Read((attributes & 0b00010000) > 0 ? 0xFF49 : 0xFF48); //0xFF49 OBP1, 0xFF48 OBP0

	u16 tileAddress = 0x8000 + tileIndex * 16;

	for (u8 line = 0; line < spriteHeight; line++) {
        u8 spriteLine = flipY ? spriteHeight - 1 - line : line;
		u8 lowByte = mmu.Read(tileAddress + spriteLine * 2);
		u8 highByte = mmu.Read(tileAddress + spriteLine * 2 + 1);

        u16 screenPosBase = (spriteY + line - 1) * (256 + 8) + spriteX;

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = flipX ? 7 - bit : bit;
            u16 screenPos = screenPosBase + (7 - bit);
            
			u8 lowBit = (lowByte >> pixel) & 0x01;
			u8 highBit = (highByte >> pixel) & 0x01;
			u8 index = lowBit | (highBit << 1);

            if (index > 0)
                SetPixel(screenPos, (palette >> (index << 1)) & 0x03);
        }
	}
}

void SpritesViewer::SetPixel(unsigned int pixelIndex, u8 gbColor) {
	u8 gpuColor = 255 - gbColor * 85;
	screenArray[pixelIndex * 4] = gpuColor;
	screenArray[pixelIndex * 4 + 1] = gpuColor;
	screenArray[pixelIndex * 4 + 2] = gpuColor;
	screenArray[pixelIndex * 4 + 3] = 0xFF;
}
