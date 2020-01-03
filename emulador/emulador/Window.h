#pragma once

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <string>
#include <memory>

class Window {
public:

	Window(unsigned int Width, unsigned int Height, const std::string& Title);
	virtual ~Window();

	std::unique_ptr<sf::Uint8[]> screenArray;
	sf::Texture screenTexture;
	sf::Sprite screenSprite;

	std::unique_ptr<sf::RenderWindow> renderWindow;
};
