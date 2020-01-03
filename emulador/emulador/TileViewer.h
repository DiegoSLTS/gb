#pragma once

#include "Types.h"
#include "Window.h"

class MMU;

class TileViewer : public Window {
public:
	TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU& Mmu, u16 Address);
	virtual ~TileViewer();

	void Update();

protected:
	MMU& mmu;
	u16 address = 0;

	virtual u16 GetTileAddress(u8 x, u8 y) const;

private:
	u8 tilesPerRow = 0;
	u8 rows = 0;

	void UpdateTile(u8 x, u8 y);
	void SetPixel(unsigned int pixelIndex, u8 gbColor);
};