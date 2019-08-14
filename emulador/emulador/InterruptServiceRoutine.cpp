#include "InterruptServiceRoutine.h"
#include "MMU.h"

u8 InterruptServiceRoutine::Read(u16 address) {
	switch (address) {
	case 0xFFFF:
		return IE;
	case 0xFF0F:
		return IF | 0b11100000;
	}
	return 0xFF;
}

void InterruptServiceRoutine::Write(u8 value, u16 address) {
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

bool InterruptServiceRoutine::IsInterruptEnabled(u8 interruptPosition) {
	return IE & (1 << interruptPosition);
}

bool InterruptServiceRoutine::IsInterruptSet(u8 interruptPosition) {
	return IF & (1 << interruptPosition);
}
