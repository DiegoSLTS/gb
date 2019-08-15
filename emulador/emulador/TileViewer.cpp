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

	for (int i = 0; i < 8; i++) {
		u8 tileDataLow = mmu->Read(tileDataAddress + i * 2);
		u8 tileDataHigh = mmu->Read(tileDataAddress + i * 2 + 1);
		
		//TODO reuse code from GPU
		u8 p7r0id = (tileDataLow & 0x80) >> 7 | (tileDataHigh & 0x80) >> 6;
		u8 p6r0id = (tileDataLow & 0x40) >> 6 | (tileDataHigh & 0x40) >> 5;
		u8 p5r0id = (tileDataLow & 0x20) >> 5 | (tileDataHigh & 0x20) >> 4;
		u8 p4r0id = (tileDataLow & 0x10) >> 4 | (tileDataHigh & 0x10) >> 3;
		u8 p3r0id = (tileDataLow & 0x08) >> 3 | (tileDataHigh & 0x08) >> 2;
		u8 p2r0id = (tileDataLow & 0x04) >> 2 | (tileDataHigh & 0x04) >> 1;
		u8 p1r0id = (tileDataLow & 0x02) >> 1 | (tileDataHigh & 0x02);
		u8 p0r0id = (tileDataLow & 0x01) | (tileDataHigh & 0x01) << 1;

		u16 screenPosBase = (y * 8 + i) * screenTexture.getSize().x + x * 8;

		SetPixel(screenPosBase, (bgPalette & (paletteMask << (p7r0id << 1))) >> (p7r0id << 1));
		SetPixel(screenPosBase + 1, (bgPalette & (paletteMask << (p6r0id << 1))) >> (p6r0id << 1));
		SetPixel(screenPosBase + 2, (bgPalette & (paletteMask << (p5r0id << 1))) >> (p5r0id << 1));
		SetPixel(screenPosBase + 3, (bgPalette & (paletteMask << (p4r0id << 1))) >> (p4r0id << 1));
		SetPixel(screenPosBase + 4, (bgPalette & (paletteMask << (p3r0id << 1))) >> (p3r0id << 1));
		SetPixel(screenPosBase + 5, (bgPalette & (paletteMask << (p2r0id << 1))) >> (p2r0id << 1));
		SetPixel(screenPosBase + 6, (bgPalette & (paletteMask << (p1r0id << 1))) >> (p1r0id << 1));
		SetPixel(screenPosBase + 7, (bgPalette & (paletteMask << (p0r0id << 1))) >> (p0r0id << 1));
	}
}

void TileViewer::SetPixel(unsigned int pixelIndex, u8 gbColor) {
	u8 gpuColor = 255 - gbColor * 85;
	screenArray[pixelIndex * 4] = gpuColor;
	screenArray[pixelIndex * 4 + 1] = gpuColor;
	screenArray[pixelIndex * 4 + 2] = gpuColor;
	screenArray[pixelIndex * 4 + 3] = 0xFF;
}
