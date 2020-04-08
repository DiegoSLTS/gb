#include "GameWindow.h"
#include "MMU.h"
#include "GPU.h"

#include <iostream>

GameWindow::GameWindow(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GPU& Gpu, const std::string& RomName)
	: Window(Width, Height, Title, Position), gpu(Gpu), romName(RomName) {
    screenshotTexture.create(width * 2, height * 2);
    screenShotData = new sf::Uint32[width * 2 * height * 2];
}
GameWindow::~GameWindow() {
    delete[] screenShotData;
}

void GameWindow::Clear() {
    memset(screenArray, 0, width * height * 4);
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

void GameWindow::TakeScreenshot() {
    sf::Uint32* originalPixels = ((sf::Uint32*)screenArray);
    const u16 w = width * 2;
    for (u8 y = 0; y < height; y++) {
        for (u8 x = 0; x < width; x++) {
            u32 srcIndex = y * width + x;
            u32 destIndex = (y * w + x) * 2;
            sf::Uint32 color = originalPixels[srcIndex];
            screenShotData[destIndex] = color;
            screenShotData[destIndex + 1] = color;
            screenShotData[destIndex + w] = color;
            screenShotData[destIndex + w + 1] = color;
        }
    }
    screenshotTexture.update((sf::Uint8*)screenShotData);
    screenshotTexture.copyToImage().saveToFile(romName + "_" + std::to_string(screenshotsCount) + ".jpg");
    screenshotsCount++;
}
