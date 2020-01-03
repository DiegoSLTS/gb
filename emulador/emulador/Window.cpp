#include "Window.h"

Window::Window(unsigned int Width, unsigned int Height, const std::string& Title) {
	screenArray = std::make_unique<sf::Uint8[]>(Width * Height * 4);

	screenTexture.create(Width, Height);
	screenTexture.update(screenArray.get());

	screenSprite.setTexture(screenTexture, true);
	screenSprite.setPosition(0.0f, 0.0f);

	renderWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode(Width, Height), Title);
	sf::Vector2u s(Width * 2, Height * 2);
	renderWindow->setSize(s);
	renderWindow->setVerticalSyncEnabled(false);
	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

Window::~Window() {}