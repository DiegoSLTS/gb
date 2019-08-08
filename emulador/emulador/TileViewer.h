#pragma once

#include "Window.h"

class MMU;

class TileViewer : public Window {
public:
	TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, uint16_t Address);
	virtual ~TileViewer();

	MMU* mmu = nullptr;
	uint16_t address = 0;

	uint8_t tilesPerRow = 0;
	uint8_t rows = 0;

	void Update();
	void UpdateTile(uint8_t x, uint8_t y);
	void SetPixel(unsigned int pixelIndex, uint8_t gbColor);
	virtual uint16_t GetTileAddress(uint8_t x, uint8_t y) const;
};