#pragma once

#include "Types.h"
#include "Window.h"

class GameBoy;

class TileViewer : public Window {
public:
	TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy);
	virtual ~TileViewer();

	void Update();

    void NextPalette();
    void PreviousPalette();

    void ToggleBank();

protected:
	GameBoy& gameBoy;

	virtual u16 GetTileAddress(u8 x, u8 y) const;
    virtual void UpdateTile(u8 x, u8 y);

    bool isCGB = false;

private:
	u8 tilesPerRow = 0;
	u8 rows = 0;
    
    void UpdateTitle();

    u8 cgbPaletteIndex = 0;
    u8 VRAMBank = 0;
};
