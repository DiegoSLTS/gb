#include "MMU.h"

#include "BootRom.h"
#include "IAddressable.h"

uint8_t MMU::Read(uint16_t address) {
	IAddressable* addressable = GetAddresableFor(address);
	//TODO unused bits set to 1, except IE register
	if (addressable != nullptr)
		return addressable->Read(address);
	
	if (address < 0x100 && IsBootRomEnabled()) { //bios
		return bootRom[address];
	} else if (address < 0x8000) { //cart
		// TODO MBC: https://gekkio.fi/files/gb-docs/gbctr.pdf
		if (cartridge == nullptr)
			return 0xFF;
		else
			return cartridge->Read(address);
	} else if (address < 0xA000) { // video ram
		return videoRAM[address - 0x8000];
	} else if (address < 0xC000) { // external (cart) ram
		return cartridge->Read(address);
	} else if (address < 0xE000) { // internal ram
		return internalRAM[address - 0xC000];
	} else if (address < 0xFE00) { // internal ram copy
		return internalRAM[address - 0xE000];
	} else if (address < 0xFEA0) { // sprites (OAM)
		return oam[address - 0xFE00];
	} else if (address < 0xFF00) {
		return 0; //TODO // unused
	} else if (address < 0xFF80) { // I/O
		return ioPorts[address - 0xFF00];
	} else { // Zero page RAM (HRAM)
		return zeroPageRAM[address - 0xFF80];
	}
}

void MMU::Write(uint16_t address, uint8_t value) {
	IAddressable* addressable = GetAddresableFor(address);
	if (addressable != nullptr) {
		addressable->Write(value, address);
		return;
	}

	if (address < 0x256 && IsBootRomEnabled()) { //bios
		//TODO ignore?
	} else if (address < 0x8000) { //cart
		cartridge->Write(value, address);
	} else if (address < 0xA000) { // vram
		videoRAM[address - 0x8000] = value;
	} else if (address < 0xC000) { // external (cart) ram
		cartridge->Write(value, address);
	} else if (address < 0xE000) { // internal ram
		internalRAM[address - 0xC000] = value;
	} else if (address < 0xFE00) { // internal ram copy
		internalRAM[address - 0xE000] = value;
	} else if (address < 0xFEA0) { // sprites (OAM)
		oam[address - 0xFE00] = value;
	} else if (address < 0xFF00) { // unused
		//TODO ignore?
	} else if (address < 0xFF80) { // I/O
		ioPorts[address - 0xFF00] = value;
	} else { // Zero page RAM
		zeroPageRAM[address - 0xFF80] = value;
	}
}

void MMU::WriteBit(uint16_t address, uint8_t bitPosition, bool set) {
	uint8_t reg = Read(address);
	if (set)
		reg |= (1 << bitPosition);
	else
		reg &= ~(1 << bitPosition);
	Write(address, reg);
}

bool MMU::IsBootRomEnabled() {
	return Read(0xFF50) == 0;
}

void MMU::ResetInterruptFlag(uint8_t interruptPosition) {
	WriteBit(0xFF0F, interruptPosition, false);
}

void MMU::SetInterruptFlag(uint8_t interruptPosition) {
	WriteBit(0xFF0F, interruptPosition, true);
}

IAddressable* MMU::GetAddresableFor(uint16_t address) {
	//TODO sound registers

	switch (address) {
	case 0xFF40:
	case 0xFF41:
	case 0xFF42:
	case 0xFF43:
	case 0xFF44:
	case 0xFF45:
	case 0xFF47:
	case 0xFF48:
	case 0xFF49:
	case 0xFF4A:
	case 0xFF4B:
		return gpu;
	case 0xFF46:
		return dma;
	case 0xFF00:
		return joypad;
	case 0xFF04:
	case 0xFF05:
	case 0xFF06:
	case 0xFF07:
		return timer;
	case 0xFF01:
	case 0xFF02:
		return serial;
	case 0xFF0F:
	case 0xFFFF:
		return interruptServiceRoutine;
	}

	return nullptr;
}

void MMU::Copy(uint16_t from, uint16_t to) {
	Write(to, Read(from));
}

void MMU::Load(std::ifstream& stream) const {
	stream.read((char*)internalRAM, 8192);
	stream.read((char*)ioPorts, 128);
	stream.read((char*)oam, 160);
	stream.read((char*)videoRAM, 8192);
	stream.read((char*)zeroPageRAM, 128);
}

void MMU::Save(std::ofstream& stream) const {
	stream.write((const char*)internalRAM, 8192);
	stream.write((const char*)ioPorts, 128);
	stream.write((const char*)oam, 160);
	stream.write((const char*)videoRAM, 8192);
	stream.write((const char*)zeroPageRAM, 128);
}
