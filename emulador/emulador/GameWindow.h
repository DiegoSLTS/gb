#pragma once

#include "Types.h"
#include "Window.h"

class GPU;

class GameWindow : public Window {
public:
    GameWindow(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GPU& Gpu, const std::string& RomName);
    virtual ~GameWindow();

    void Update();

    void Clear();

    void DrawLine(u8 line);

    void TakeScreenshot();
private:
    GPU& gpu;

    sf::Texture screenshotTexture;
    sf::Uint32* screenShotData = nullptr;
    u16 screenshotsCount = 0;
    const std::string& romName;
};
