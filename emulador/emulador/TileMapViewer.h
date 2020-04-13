#pragma once

#include "TileViewer.h"
#include "GPU.h"

class GameBoy;

class TileMapViewer : public TileViewer {
public:
	TileMapViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy, u8 mapNumber);
	virtual ~TileMapViewer();

    void PrintTile(u8 x, u8 y);
    void OnMouseClicked(u32 x, u32 y);

protected:
	u16 GetTileAddress(u8 x, u8 y) const;
    virtual void UpdateTile(u8 x, u8 y) override;

private:
    u8 mapNumber = 0;
    u16 address = 0;
	LCDC_t& LCDC;

    u8 loggedTileX = 255;
    u8 loggedTileY = 255;
};
