#include "GameWindow.h"
#include "MMU.h"
#include "GPU.h"

#include <iostream>

GameWindow::GameWindow(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GPU& Gpu)
	: Window(Width, Height, Title, Position), gpu(Gpu) {}
GameWindow::~GameWindow() {}

void GameWindow::Clear() {
    memset(screenArray, 0, 160 * 144 * 4);
}

void GameWindow::Update() {
    screenTexture.update(screenArray);
    screenSprite.setTexture(screenTexture, true);

    renderWindow->clear();
    renderWindow->draw(screenSprite);
    renderWindow->display();
}

void GameWindow::DrawLine(u8 line) {
    u32 x = line * width;
	u32 xMax = x + width;
    for (; x < xMax; x++)
        ((sf::Uint32*)screenArray)[x] = gpu.GetABGRAt(x).v;
}
