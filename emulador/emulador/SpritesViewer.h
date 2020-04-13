#pragma once

#include "Types.h"
#include "Window.h"
#include "GPU.h"

class GameBoy;

class SpritesViewer : public Window {
public:
	SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy);
	virtual ~SpritesViewer();

	void Update();

	void ToggleBackground();

    void OnMouseClicked(u32 x, u32 y);

private:
	GameBoy& gameBoy;

	void DrawSprite(u8 index);

    bool isCGB = false;
	LCDC_t& LCDC;
	u8* oam = nullptr;

	u8 clearColor = 0x00;

    void PrintSprite(u8 index);

    u8 loggedSprite = 255;
};
