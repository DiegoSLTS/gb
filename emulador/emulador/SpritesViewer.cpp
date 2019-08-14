#include "SpritesViewer.h"
#include "MMU.h"
#include "GPU.h"

SpritesViewer::SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu) : Window(Width, Height, Title), mmu(Mmu) {
	Update();
}

SpritesViewer::~SpritesViewer() {
	mmu = nullptr;
}

void SpritesViewer::Update() {
	memset(screenArray, 255, (256 + 8)*(256 + 16) * 4);
	for (uint8_t i = 0; i < 40; i++)
		DrawSprite(i);

	screenTexture.update(screenArray);
	screenSprite.setTexture(screenTexture, true);

	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

void SpritesViewer::DrawSprite(uint8_t index) {
	const uint8_t paletteMask = 0b00000011;

	uint8_t LCDC = mmu->Read(0xFF40);
	uint8_t spriteHeight = LCDC & LCDCMask::ObjSize ? 16 : 8;
	uint8_t spriteIndex = index * 4;

	uint8_t spriteY = mmu->Read(0xFE00 + spriteIndex);
	uint8_t spriteX = mmu->Read(0xFE00 + spriteIndex + 1);
	uint8_t tileIndex = mmu->Read(0xFE00 + spriteIndex + 2);
	uint8_t attributes = mmu->Read(0xFE00 + spriteIndex + 3);

	bool hasPriority = (attributes & 0b10000000) > 0;
	bool flipY = (attributes & 0b01000000) > 0;
	bool flipX = (attributes & 0b00100000) > 0;
	uint16_t palette = mmu->Read((attributes & 0b00010000) > 0 ? 0xFF49 : 0xFF48); //0xFF49 OBP1, 0xFF48 OBP0

	uint16_t tileAddress = 0x8000 + tileIndex * 16;

	for (uint8_t line = 0; line < spriteHeight; line++) {
		uint8_t tileDataLow = mmu->Read(tileAddress + line * 2);
		uint8_t tileDataHigh = mmu->Read(tileAddress + line * 2 + 1);

		//TODO reuse code from GPU
		uint8_t p7id = (tileDataLow & 0x80) >> 7 | (tileDataHigh & 0x80) >> 6;
		uint8_t p6id = (tileDataLow & 0x40) >> 6 | (tileDataHigh & 0x40) >> 5;
		uint8_t p5id = (tileDataLow & 0x20) >> 5 | (tileDataHigh & 0x20) >> 4;
		uint8_t p4id = (tileDataLow & 0x10) >> 4 | (tileDataHigh & 0x10) >> 3;
		uint8_t p3id = (tileDataLow & 0x08) >> 3 | (tileDataHigh & 0x08) >> 2;
		uint8_t p2id = (tileDataLow & 0x04) >> 2 | (tileDataHigh & 0x04) >> 1;
		uint8_t p1id = (tileDataLow & 0x02) >> 1 | (tileDataHigh & 0x02);
		uint8_t p0id = (tileDataLow & 0x01) | (tileDataHigh & 0x01) << 1;

		uint16_t screenPosBase = (spriteY + line) * (256 + 8) + spriteX;

		if (p7id != 0)
			SetPixel(screenPosBase, (palette & (paletteMask << (p7id << 1))) >> (p7id << 1));
		if (p6id != 0)
			SetPixel(screenPosBase + 1, (palette & (paletteMask << (p6id << 1))) >> (p6id << 1));
		if (p5id != 0)
			SetPixel(screenPosBase + 2,(palette & (paletteMask << (p5id << 1))) >> (p5id << 1));
		if (p4id != 0)
			SetPixel(screenPosBase + 3, (palette & (paletteMask << (p4id << 1))) >> (p4id << 1));
		if (p3id != 0)
			SetPixel(screenPosBase + 4, (palette & (paletteMask << (p3id << 1))) >> (p3id << 1));
		if (p2id != 0)
			SetPixel(screenPosBase + 5, (palette & (paletteMask << (p2id << 1))) >> (p2id << 1));
		if (p1id != 0)
			SetPixel(screenPosBase + 6, (palette & (paletteMask << (p1id << 1))) >> (p1id << 1));
		if (p0id != 0)
			SetPixel(screenPosBase + 7, (palette & (paletteMask << (p0id << 1))) >> (p0id << 1));
	}
}

void SpritesViewer::SetPixel(unsigned int pixelIndex, uint8_t gbColor) {
	uint8_t gpuColor = 255 - gbColor * 85;
	screenArray[pixelIndex * 4] = gpuColor;
	screenArray[pixelIndex * 4 + 1] = gpuColor;
	screenArray[pixelIndex * 4 + 2] = gpuColor;
	screenArray[pixelIndex * 4 + 3] = 0xFF;
}
