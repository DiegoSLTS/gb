#pragma once

#include "Types.h"
#include "Window.h"

class MMU;

class TileViewer : public Window {
public:
	TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, u16 Address);
	virtual ~TileViewer();

	MMU* mmu = nullptr;
	u16 address = 0;

	u8 tilesPerRow = 0;
	u8 rows = 0;

	void Update();
	void UpdateTile(u8 x, u8 y);
	void SetPixel(unsigned int pixelIndex, u8 gbColor);
	virtual u16 GetTileAddress(u8 x, u8 y) const;
};