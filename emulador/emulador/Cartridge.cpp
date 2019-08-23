#include "Cartridge.h"

#include <fstream>

Cartridge::Cartridge(const std::string& romPath) {
    LoadFile(romPath);
}

Cartridge::~Cartridge() {}

void Cartridge::LoadFile(const std::string& path) {
	std::ifstream readStream;
	readStream.open(path, std::ios::in | std::ios::binary);
	//TODO MBCs support
	readStream.read((char*)rom, 32 * 1024);
	readStream.close();
}

u8 Cartridge::Read(u16 address) {
	if (address >= 0x8000)
		return 0xFF;
	//TODO MBCs support
	return rom[address];
}

void Cartridge::Write(u8 value, u16 address) {
	//TODO MBCs support
}

void Cartridge::Load(std::ifstream& stream) const {
	//TODO MBCs support
}

void Cartridge::Save(std::ofstream& stream) const {
	//TODO MBCs support
}