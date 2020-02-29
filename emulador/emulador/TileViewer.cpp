#include "TileViewer.h"
#include "MMU.h"
#include "GPU.h"

TileViewer::TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, MMU& Mmu, u16 Address) : Window(Width, Height, Title, Position, false), mmu(Mmu), address(Address) {
	tilesPerRow = Width / 8;
	rows = Height / 8;
	Update();
}

TileViewer::~TileViewer() {}

void TileViewer::Update() {
    if (!IsOpen())
        return;

	for (u8 y = 0; y < rows; y++)
		for (u8 x = 0; x < tilesPerRow; x++)
			UpdateTile(x, y);

	screenTexture.update(screenArray);
	screenSprite.setTexture(screenTexture, true);

	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

u16 TileViewer::GetTileAddress(u8 x, u8 y) const {
	return address + (y * tilesPerRow + x) * 16;
}

void TileViewer::UpdateTile(u8 x, u8 y) {
	u8 bgPalette = mmu.Read(0xFF47);

	u16 tileDataAddress = GetTileAddress(x, y);

	for (int line = 0; line < 8; line++) {
		u8 lowByte = mmu.Read(tileDataAddress + line * 2);
		u8 highByte = mmu.Read(tileDataAddress + line * 2 + 1);
		
        u16 screenPosBase = (y * 8 + line) * screenTexture.getSize().x + x * 8;

        for (s8 pixel = 7; pixel >= 0; pixel--) {
            u16 screenPos = screenPosBase + (7 - pixel);
			u8 lowBit = (lowByte >> pixel) & 0x01;
			u8 highBit = (highByte >> pixel) & 0x01;
			u8 index = lowBit | (highBit << 1);

            SetPixel(screenPos, (bgPalette >> (index << 1)) & 0x03);
        }
	}
}

void TileViewer::SetPixel(unsigned int pixelIndex, u8 gbColor) {
    // turn gpuScreen value [0,3] into an 8 bit value [255,0], 85 == 255/3
    const static u8 sfmlColors[] = { 0xFF, 0xAA , 0x55, 0x00 }; // 255 - index * 85 

    u8 sfmlColor = sfmlColors[gbColor];
    screenArray[pixelIndex * 4] = sfmlColor;
    screenArray[pixelIndex * 4 + 1] = sfmlColor;
    screenArray[pixelIndex * 4 + 2] = sfmlColor;
    screenArray[pixelIndex * 4 + 3] = 0xFF;
}
