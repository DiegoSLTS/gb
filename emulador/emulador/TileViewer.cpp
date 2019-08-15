#include "TileViewer.h"
#include "MMU.h"
#include "GPU.h"

TileViewer::TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, u16 Address) : Window(Width,Height,Title), mmu(Mmu), address(Address) {
	tilesPerRow = Width / 8;
	rows = Height / 8;
	Update();
}

TileViewer::~TileViewer() {
	mmu = nullptr;
}

void TileViewer::Update() {
	for (u8 y = 0; y < rows; y++) {
		for (u8 x = 0; x < tilesPerRow; x++) {
			UpdateTile(x, y);
		}
	}

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
	const u8 paletteMask = 0b00000011;
	u8 bgPalette = mmu->Read(0xFF47);

	u16 tileDataAddress = GetTileAddress(x, y);

	for (int line = 0; line < 8; line++) {
		u8 tileDataLow = mmu->Read(tileDataAddress + line * 2);
		u8 tileDataHigh = mmu->Read(tileDataAddress + line * 2 + 1);
		
        u16 screenPosBase = (y * 8 + line) * screenTexture.getSize().x + x * 8;

        for (s8 pixel = 7; pixel >= 0; pixel--) {
            u16 screenPos = screenPosBase + (7 - pixel);
            u8 lowBit = (tileDataLow >> pixel) & 0x01;
            u8 highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
            u8 id = lowBit | highBit;

            SetPixel(screenPos, (bgPalette & (paletteMask << (id << 1))) >> (id << 1));
        }
	}
}

void TileViewer::SetPixel(unsigned int pixelIndex, u8 gbColor) {
	u8 gpuColor = 255 - gbColor * 85;
	screenArray[pixelIndex * 4] = gpuColor;
	screenArray[pixelIndex * 4 + 1] = gpuColor;
	screenArray[pixelIndex * 4 + 2] = gpuColor;
	screenArray[pixelIndex * 4 + 3] = 0xFF;
}
