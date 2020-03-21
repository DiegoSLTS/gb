#include "SpritesViewer.h"
#include "GameBoy.h"
#include "MMU.h"
#include "GPU.h"

SpritesViewer::SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy)
	: Window(Width, Height, Title, Position, false), gameBoy(GameBoy), LCDC(gameBoy.gpu.GetLCDCRef()) {
	oam = gameBoy.gpu.GetOAMPtr();
	isCGB = gameBoy.IsCGB;

	Update();
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
	u8 spriteHeight = LCDC.spritesSize == 0 ? 8 : 16;
	u8 spriteIndex = index * 4;

	u8 spriteY = oam[spriteIndex];
	u8 spriteX = oam[spriteIndex + 1];
	u8 tileIndex = oam[spriteIndex + 2];
	OBJAttributes attributes = { oam[spriteIndex + 3] };

	u16 tileAddress = 0x8000 + tileIndex * 16;

	for (u8 line = 0; line < spriteHeight; line++) {
        u8 spriteLine = attributes.flipY ? spriteHeight - 1 - line : line;
		u8 lowByte = gameBoy.gpu.ReadVRAM(tileAddress + spriteLine * 2, attributes.bank);
		u8 highByte = gameBoy.gpu.ReadVRAM(tileAddress + spriteLine * 2 + 1, attributes.bank);

        u16 screenPosBase = (spriteY + line - 1) * (256 + 8) + spriteX;

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = attributes.flipX ? 7 - bit : bit;
            u16 screenPos = screenPosBase + (7 - bit);
            
			u8 lowBit = (lowByte >> pixel) & 0x01;
			u8 highBit = (highByte >> pixel) & 0x01;
			u8 index = lowBit | (highBit << 1);

            if (index > 0) {
				PixelInfo pixelInfo = { 0 };
				pixelInfo.colorIndex = index;
				pixelInfo.paletteIndex = isCGB ? attributes.paletteIndex : attributes.palette;

				((sf::Uint32*)screenArray)[screenPos] = gameBoy.gpu.GetABGR(pixelInfo).v;
            }
        }
	}
}
