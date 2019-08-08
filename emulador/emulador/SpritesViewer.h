#pragma once

#include "Window.h"

class MMU;

class SpritesViewer : public Window {
public:
	SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU* Mmu);
	virtual ~SpritesViewer();

	MMU* mmu = nullptr;

	void Update();
	void DrawSprite(uint8_t index);
	void SetPixel(unsigned int pixelIndex, uint8_t gbColor);
};