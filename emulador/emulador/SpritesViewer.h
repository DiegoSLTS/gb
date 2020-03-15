#pragma once

#include "Types.h"
#include "Window.h"

class GameBoy;

class SpritesViewer : public Window {
public:
	SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy);
	virtual ~SpritesViewer();

	void Update();

private:
	GameBoy& gameBoy;

	void DrawSprite(u8 index);

    bool isCGB = false;
};
