#pragma once

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <string>

class Window {
public:

	Window(unsigned int Width, unsigned int Height, const std::string& Title);
	virtual ~Window();

	sf::Uint8* screenArray = nullptr;
	sf::Texture screenTexture;
	sf::Sprite screenSprite;

	sf::RenderWindow* renderWindow = nullptr;
};
