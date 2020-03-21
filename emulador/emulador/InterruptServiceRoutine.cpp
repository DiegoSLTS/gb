#include "InterruptServiceRoutine.h"

InterruptServiceRoutine::InterruptServiceRoutine() {}
InterruptServiceRoutine::~InterruptServiceRoutine() {}

u8 InterruptServiceRoutine::Read(u16 address) {
	switch (address) {
	case 0xFFFF:
		return IE | 0xE0;
	case 0xFF0F:
		return IF | 0xE0;
	}
	return 0xFF;
}

void InterruptServiceRoutine::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFFFF:
		IE = value | 0xE0; break;
	case 0xFF0F:
		IF = value | 0xE0; break;
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

bool InterruptServiceRoutine::IsInterruptEnabled(u8 interruptPosition) {
	return ((IE & (1 << interruptPosition)) != 0);
}

bool InterruptServiceRoutine::IsInterruptSet(u8 interruptPosition) {
	return ((IF & (1 << interruptPosition)) != 0);
}
