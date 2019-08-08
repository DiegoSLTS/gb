#include "TileViewer.h"
#include "MMU.h"
#include "GPU.h"

TileViewer::TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, uint16_t Address) : Window(Width,Height,Title), mmu(Mmu), address(Address) {
	tilesPerRow = Width / 8;
	rows = Height / 8;
	Update();
}

TileViewer::~TileViewer() {
	mmu = nullptr;
}

void TileViewer::Update() {
	for (uint8_t y = 0; y < rows; y++) {
		for (uint8_t x = 0; x < tilesPerRow; x++) {
			UpdateTile(x, y);
		}
	}

	screenTexture.update(screenArray);
	screenSprite.setTexture(screenTexture, true);

	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

uint16_t TileViewer::GetTileAddress(uint8_t x, uint8_t y) const {
	return address + (y * tilesPerRow + x) * 16;
}

void TileViewer::UpdateTile(uint8_t x, uint8_t y) {
	const uint8_t paletteMask = 0b00000011;
	uint8_t bgPalette = mmu->Read(0xFF47); //TODO for sprites?

	uint16_t tileDataAddress = GetTileAddress(x, y);

	for (int i = 0; i < 8; i++) {
		uint8_t tileDataLow = mmu->Read(tileDataAddress + i * 2);
		uint8_t tileDataHigh = mmu->Read(tileDataAddress + i * 2 + 1);
		
		//TODO reuse code from GPU
		uint8_t p7r0id = (tileDataLow & 0x80) >> 7 | (tileDataHigh & 0x80) >> 6;
		uint8_t p6r0id = (tileDataLow & 0x40) >> 6 | (tileDataHigh & 0x40) >> 5;
		uint8_t p5r0id = (tileDataLow & 0x20) >> 5 | (tileDataHigh & 0x20) >> 4;
		uint8_t p4r0id = (tileDataLow & 0x10) >> 4 | (tileDataHigh & 0x10) >> 3;
		uint8_t p3r0id = (tileDataLow & 0x08) >> 3 | (tileDataHigh & 0x08) >> 2;
		uint8_t p2r0id = (tileDataLow & 0x04) >> 2 | (tileDataHigh & 0x04) >> 1;
		uint8_t p1r0id = (tileDataLow & 0x02) >> 1 | (tileDataHigh & 0x02);
		uint8_t p0r0id = (tileDataLow & 0x01) | (tileDataHigh & 0x01) << 1;

		uint16_t screenPosBase = (y * 8 + i) * screenTexture.getSize().x + x * 8;

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

void TileViewer::SetPixel(unsigned int pixelIndex, uint8_t gbColor) {
	uint8_t gpuColor = 255 - (gbColor / 3.0f) * 255;
	screenArray[pixelIndex * 4] = gpuColor;
	screenArray[pixelIndex * 4 + 1] = gpuColor;
	screenArray[pixelIndex * 4 + 2] = gpuColor;
	screenArray[pixelIndex * 4 + 3] = 0xFF;
}
