#pragma once

#include "TileViewer.h"

class MMU;

//TODO optimize using TileViewer screen pixels instead of reading tile data again for each tile

class TileMapViewer : public TileViewer {
public:
	TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu, uint16_t Address);
	virtual ~TileMapViewer();

	virtual uint16_t GetTileAddress(uint8_t x, uint8_t y) const override;
};