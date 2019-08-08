#include "InterruptServiceRoutine.h"
#include "MMU.h"

uint8_t InterruptServiceRoutine::Read(uint16_t address) {
	switch (address) {
	case 0xFFFF:
		return IE;
	case 0xFF0F:
		return IF | 0b11100000;
	}
	return 0xFF;
}

void InterruptServiceRoutine::Write(uint8_t value, uint16_t address) {
	switch (address) {
	case 0xFFFF:
		IE = value; break;
	case 0xFF0F:
		IF = value | 0b11100000; break;
	}
}

void InterruptServiceRoutine::Load(std::ifstream& stream) const {
	stream.read((char*)&IE, 1);
	stream.read((char*)&IF, 1);
	stream.read((char*)&IME, 1);
}

void InterruptServiceRoutine::Save(std::ofstream& stream) const {
	stream.write((const char*)&IE, 1);
	stream.write((const char*)&IF, 1);
	stream.write((const char*)&IME, 1);
}

bool InterruptServiceRoutine::IsInterruptEnabled(uint8_t interruptPosition) {
	return IE & (1 << interruptPosition);
}

bool InterruptServiceRoutine::IsInterruptSet(uint8_t interruptPosition) {
	return IF & (1 << interruptPosition);
}
