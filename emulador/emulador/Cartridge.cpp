#include "Cartridge.h"

#include <fstream>

void Cartridge::LoadFile(const std::string& path) {
	std::ifstream readStream;
	readStream.open(path, std::ios::in | std::ios::binary);
	//TODO MBCs support
	readStream.read((char*)rom, 32 * 1024);
	readStream.close();
}

uint8_t Cartridge::Read(uint16_t address) {
	if (address >= 0x8000)
		return 0xFF;
	//TODO MBCs support
	return rom[address];
}

void Cartridge::Write(uint8_t value, uint16_t address) {
	//TODO MBCs support
}

void Cartridge::Load(std::ifstream& stream) const {
	//TODO MBCs support
}

void Cartridge::Save(std::ofstream& stream) const {
	//TODO when MBCs are supported
}