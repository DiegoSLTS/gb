#include "Audio.h"

Audio::Audio() {}
Audio::~Audio() {}

uint8_t Audio::Read(uint16_t address) {
	if (address >= 0xFF30 && address <= 0xFF3F)
		return WaveRAM[address - 0xFF30];

	switch (address) {
	case 0xFF10:
		return NR10 | 0x80;
	case 0xFF11:
		return NR11;
	case 0xFF12:
		return NR12;
	case 0xFF13:
		return NR13;
	case 0xFF14:
		return NR14;
	case 0xFF16:
		return NR21;
	case 0xFF17:
		return NR22;
	case 0xFF18:
		return NR23;
	case 0xFF19:
		return NR24;
	case 0xFF1A:
		return NR30 | 0x7F;
	case 0xFF1B:
		return NR31;
	case 0xFF1C:
		return NR32 | 0b10011111;
	case 0xFF1D:
		return NR33;
	case 0xFF1E:
		return NR34;
	case 0xFF20:
		return NR41 | 0b11000000;
	case 0xFF21:
		return NR42;
	case 0xFF22:
		return NR43;
	case 0xFF23:
		return NR44 | 0b00111111;
	case 0xFF24:
		return NR50;
	case 0xFF25:
		return NR51;
	case 0xFF26:
		return NR52 | 0b01110000;
	}

	return 0xFF;
}

void Audio::Write(uint8_t value, uint16_t address) {
	if (address >= 0xFF30 && address <= 0xFF3F) {
		WaveRAM[address - 0xFF30] = value;
		return;
	}

	switch (address) {
	case 0xFF10:
		NR10 = value | 0x80; break;
	case 0xFF11:
		NR11 = value; break;
	case 0xFF12:
		NR12 = value; break;
	case 0xFF13:
		NR13 = value; break;
	case 0xFF14:
		NR14 = value; break;
	case 0xFF16:
		NR21 = value; break;
	case 0xFF17:
		NR22 = value; break;
	case 0xFF18:
		NR23 = value; break;
	case 0xFF19:
		NR24 = value; break;
	case 0xFF1A:
		NR30 = value | 0x7F; break;
	case 0xFF1B:
		NR31 = value; break;
	case 0xFF1C:
		NR32 = value | 0b10011111; break;
	case 0xFF1D:
		NR33 = value; break;
	case 0xFF1E:
		NR34 = value; break;
	case 0xFF20:
		NR41 = value | 0b11000000; break;
	case 0xFF21:
		NR42 = value; break;
	case 0xFF22:
		NR43 = value; break;
	case 0xFF23:
		NR44 = value | 0b00111111; break;
	case 0xFF24:
		NR50 = value; break;
	case 0xFF25:
		NR51 = value; break;
	case 0xFF26:
		NR52 = (value & 0xF0) | (NR52 & 0x0F) | 0b01110000; break;
	}
}

void Audio::Load(std::ifstream& stream) const {
	stream.read((char*)&NR10, 37);
}

void Audio::Save(std::ofstream& stream) const {
	stream.write((const char*)&NR10, 37);
}