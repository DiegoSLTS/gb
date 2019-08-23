#pragma once

#include "Types.h"
#include "Window.h"

class MMU;

class SpritesViewer : public Window {
public:
	SpritesViewer(unsigned int Width, unsigned int Height, const std::string& Title, MMU& Mmu);
	virtual ~SpritesViewer();

	MMU& mmu;

	void Update();
	void DrawSprite(u8 index);
	void SetPixel(unsigned int pixelIndex, u8 gbColor);
};