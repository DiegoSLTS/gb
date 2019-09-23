#include "Cartridge.h"

#include <fstream>

Cartridge::Cartridge(const std::string& romPath) {
    LoadFile(romPath);
}

Cartridge::~Cartridge() {
	if (mbc != nullptr)
		delete mbc;
}

void Cartridge::LoadHeader(std::ifstream& readStream) {
	readStream.seekg(0x0134);

	// copy all fields from header from stream to header struct (not just the title)
	// IMPORTANT: variables defined in the same order as the header
	readStream.read((char*)header.title, 0x014F - 0x0134 + 1);
	header.Print();

	readStream.seekg(0,std::ios_base::beg);
}

void Cartridge::LoadRom(std::ifstream& readStream) {
	switch (header.cartridgeType) {
	case 0:
	case 8:
	case 9:
		mbc = new RomOnly(header);
		break;
	case 1:
	case 2:
	case 3:
		mbc = new MBC1(header);
		break;
	case 5:
	case 6:
		mbc = new MBC2(header);
		break;
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		mbc = new MBC3(header);
		break;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		mbc = new MBC5(header);
		break;
	}

	if (mbc == nullptr) {
		printf("ERROR: cartridgeType %d not supported", (unsigned int)header.cartridgeType);
		return;
	}
	
	mbc->InitArrays();
	mbc->LoadRom(readStream);

	hasBattery = header.cartridgeType == 0x03 || header.cartridgeType == 0x06 || header.cartridgeType == 0x09
		|| header.cartridgeType == 0x0D	|| header.cartridgeType == 0x0F || header.cartridgeType == 0x10
		|| header.cartridgeType == 0x13 || header.cartridgeType == 0x17	|| header.cartridgeType == 0x1B
		|| header.cartridgeType == 0x1E || header.cartridgeType == 0xFF;
}

void Cartridge::LoadRam(std::ifstream& readStream) {
	mbc->LoadRam(readStream);
}

void Cartridge::SaveRam(std::ofstream& writeStream) {
	mbc->SaveRam(writeStream);
}

void Cartridge::LoadFile(const std::string& path) {
	std::ifstream readStream;
	readStream.open(path, std::ios::in | std::ios::binary);

	LoadHeader(readStream);
	LoadRom(readStream);
	
	readStream.close();
}

u8 Cartridge::Read(u16 address) {
	return mbc->Read(address);
}

void Cartridge::Write(u8 value, u16 address) {
	mbc->Write(value, address);
}

void Cartridge::Load(std::ifstream& stream) const {
	mbc->Load(stream);
}

void Cartridge::Save(std::ofstream& stream) const {
	mbc->Save(stream);
}