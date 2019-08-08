#include "Joypad.h"

#include <SFML\Window\Keyboard.hpp>

void Joypad::Update() {
	if (JOYP & 0b00010000) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) // A
			JOYP &= 0b11111110;
		else
			JOYP |= 0b00000001;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::B)) // B
			JOYP &= 0b11111101;
		else
			JOYP |= 0b00000010;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) // select
			JOYP &= 0b11111011;
		else
			JOYP |= 0b00000100;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) // start
			JOYP &= 0b11110111;
		else
			JOYP |= 0b00001000;
	} else if (JOYP & 0b00100000) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) // right
			JOYP &= 0b11111110;
		else
			JOYP |= 0b00000001;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) // left
			JOYP &= 0b11111101;
		else
			JOYP |= 0b00000010;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) // up
			JOYP &= 0b11111011;
		else
			JOYP |= 0b00000100;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) // down
			JOYP &= 0b11110111;
		else
			JOYP |= 0b00001000;
	} else
		JOYP = 0xFF;

	//TODO set interrupt request
}

uint8_t Joypad::Read(uint16_t address) {
	return JOYP;
}

void Joypad::Write(uint8_t value, uint16_t address) {
	JOYP = (value & 0xF0) | (JOYP & 0x0F) | 0b11000000;
	Update();
}
