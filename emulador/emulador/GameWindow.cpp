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
    unsigned int x = 0;
    for (; x < width; x++) {
        unsigned int index = x + line * width;
        ((sf::Uint32*)screenArray)[index] = gpu.GetABGR(gpu.screen[index]);
    }
}
