#include "Joypad.h"
#include "InterruptServiceRoutine.h"

#include <SFML\Window\Keyboard.hpp>

Joypad::Joypad(InterruptServiceRoutine& interruptService) : interruptService(interruptService) {}
Joypad::~Joypad() {}

void Joypad::Update() {
    u8 state = JOYP;
	JOYP |= 0x0F;
	if ((JOYP & 0x20) == 0) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) // A
			JOYP &= ~0x01;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) // B
			JOYP &= ~0x02;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) // select
			JOYP &= ~0x04;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) // start
			JOYP &= ~0x08;
	} else if ((JOYP & 0x10) == 0) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) // right
			JOYP &= ~0x01;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) // left
			JOYP &= ~0x02;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) // up
			JOYP &= ~0x04;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) // down
			JOYP &= ~0x08;
	}

    for (u8 bit = 0; bit < 4; bit++) {
        if ((state & (1 << bit)) > (JOYP & (1 << bit))) {
			interruptService.SetInterruptFlag(InterruptFlag::Joypad);
            break;
        }
    }
}

u8 Joypad::Read(u16 address) {
	return JOYP;
}

void Joypad::Write(u8 value, u16 address) {
	JOYP = (value & 0xF0) | (JOYP & 0x0F) | 0xC0;
	Update();
}
