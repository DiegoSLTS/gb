#include "Window.h"

Window::Window(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i Position, bool Open, bool scale)
	: width(Width), height(Height), title(Title), position(Position), scale(scale) {
	screenArray = new sf::Uint8[width * height * 4];

	screenTexture.create(width, height);
	screenTexture.update(screenArray);

	screenSprite.setTexture(screenTexture, true);
	screenSprite.setPosition(0.0f, 0.0f);

    renderWindow = new sf::RenderWindow(sf::VideoMode(width, height), title);
    Initialize();

    if (!Open)
        Close();
}

Window::~Window() {
    Close();
    delete[] screenArray;
    screenArray = nullptr;
    delete renderWindow;
    renderWindow = nullptr;
}

void Window::Toggle() {
    if (renderWindow->isOpen())
        Close();
    else
        Open();
}

void Window::Open() {
    if (!renderWindow->isOpen()) {
        renderWindow->create(sf::VideoMode(width, height), title);
        Initialize();
    }
}

void Window::Close() {
    if (renderWindow->isOpen())
        renderWindow->close();
}

bool Window::IsOpen() const {
    return renderWindow->isOpen();
}

void Window::SetPosition(const sf::Vector2i& NewPosition) {
    position = NewPosition;
    renderWindow->setPosition(position);
}

void Window::GetFocus() {
    renderWindow->requestFocus();
}

bool Window::PollEvent(sf::Event& event) {
    return renderWindow->pollEvent(event);
}

void Window::Initialize() {
    renderWindow->setPosition(position);
	if (scale) {
		sf::Vector2u s(width * 2, height * 2);
		renderWindow->setSize(s);
	}
    renderWindow->setVerticalSyncEnabled(false);
}

void Window::SetTitle(const std::string& newTitle) {
    renderWindow->setTitle(newTitle);
}
