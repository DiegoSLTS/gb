#include "GameWindow.h"
#include "MMU.h"
#include "GPU.h"

#include <iostream>

GameWindow::GameWindow(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GPU& Gpu) : Window(Width, Height, Title, Position), gpu(Gpu) {
    memset(previousState, 0xFF, 160 * 144);
}
GameWindow::~GameWindow() {}

void GameWindow::Update() {
    // turn gpuScreen value [0,3] into an 8 bit value [255,0], 85 == 255/3
    const static u8 sfmlColors[] = { 0xFF, 0xAA , 0x55, 0x00 }; // 255 - index * 85 

    unsigned int index = 0;
    bool screenChanged = false;
    for (; index < width * height; index++) {
        if (previousState[index] == gpu.screen[index])
            continue;

        u8 gpuColor = gpu.screen[index];
        u8 sfmlColor = sfmlColors[gpuColor];
        screenArray[index * 4] = sfmlColor;
        screenArray[index * 4 + 1] = sfmlColor;
        screenArray[index * 4 + 2] = sfmlColor;
        screenArray[index * 4 + 3] = 0xFF;
        previousState[index] = gpuColor;
        screenChanged = true;
    }

    if (screenChanged) {
        screenTexture.update(screenArray);
        screenSprite.setTexture(screenTexture, true);

        renderWindow->clear();
        renderWindow->draw(screenSprite);
        renderWindow->display();
    }
}
