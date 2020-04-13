#include "InterruptServiceRoutine.h"
#include "Logger.h"

InterruptServiceRoutine::InterruptServiceRoutine() {}
InterruptServiceRoutine::~InterruptServiceRoutine() {}

u8 InterruptServiceRoutine::Read(u16 address) {
	switch (address) {
	case 0xFFFF:
		return IE;
	case 0xFF0F:
		return IF | 0xE0;
	}
	return 0xFF;
}

void InterruptServiceRoutine::Write(u8 value, u16 address) {
	switch (address) {
	case 0xFFFF:
		IE = value; break;
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

bool InterruptServiceRoutine::IsInterruptEnabled(InterruptFlag flag) {
	return ((IE & (u8)flag) != 0);
}

bool InterruptServiceRoutine::IsInterruptSet(InterruptFlag flag) {
	return ((IF & (u8)flag) != 0);
}

void InterruptServiceRoutine::ResetInterruptFlag(InterruptFlag flag) {
	IF &= ~(u8)flag;
    if (log) Logger::instance->log("Reset flag " + Logger::u8ToHex((u8)flag) + "\n");
}

void InterruptServiceRoutine::SetInterruptFlag(InterruptFlag flag) {
	IF |= (u8)flag;
    if (log) Logger::instance->log("Set flag " + Logger::u8ToHex((u8)flag) + "\n");
}
