#pragma once

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <string>
#include <memory>

class Window {
public:

	Window(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i Position, bool Open = true, bool scale = true);
	virtual ~Window();

    bool IsOpen() const;
    void Open();
    void Close();
    void Toggle();
    void GetFocus();
    void SetPosition(const sf::Vector2i& NewPosition);
    bool PollEvent(sf::Event& event);

    void SetTitle(const std::string& newTitle);

protected:
    unsigned width = 0;
    unsigned height = 0;
    std::string title;
    sf::Vector2i position;

    sf::Uint8* screenArray = nullptr;
    sf::Texture screenTexture;
    sf::Sprite screenSprite;

    sf::RenderWindow* renderWindow;

	bool scale = true;

private:
    void Initialize();
};
