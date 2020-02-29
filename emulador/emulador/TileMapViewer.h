#pragma once

#include "TileViewer.h"

class MMU;

//TODO optimize using TileViewer screen pixels instead of reading tile data again for each tile

class TileMapViewer : public TileViewer {
public:
	TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, MMU& Mmu, u16 Address);
	virtual ~TileMapViewer();

protected:
	virtual u16 GetTileAddress(u8 x, u8 y) const override;
};
