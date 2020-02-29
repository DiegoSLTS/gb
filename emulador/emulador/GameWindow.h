#pragma once

#include "Types.h"
#include "Window.h"

class GPU;

class GameWindow : public Window {
public:
    GameWindow(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GPU& Gpu);
    virtual ~GameWindow();

    void Update();

private:
    GPU& gpu;
    u8 previousState[160 * 144];
};
