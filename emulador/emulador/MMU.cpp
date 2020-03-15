#include "MMU.h"

//#include "BootRom.h"
#include "IAddressable.h"

#include <iostream>
#include "RomParser.h"

MMU::MMU() {}

MMU::~MMU() {
    if (bootRom != nullptr)
        delete[] bootRom;
}

void MMU::LoadBootRom(bool isCGB) {
    IsCGB = isCGB;
    const char* romFileName = IsCGB ? "cgb_bios.bin" : "dmg_boot.bin";

    std::ifstream readStream;
    readStream.open(romFileName, std::ios::in | std::ios::binary);

    if (readStream.fail()) {
        char errorMessage[256];
        strerror_s(errorMessage, 256);
        std::cout << "ERROR: Could not open bootrom file " << romFileName << " - Error: " << errorMessage << std::endl;
    }

    struct stat bootRomStats;
    stat(romFileName, &bootRomStats);
    bootRom = new u8[bootRomStats.st_size]{ 0 };
    readStream.read((char*)bootRom, bootRomStats.st_size);

    /*RomParser parser;
    parser.ParseBiosROM(bootRom, bootRomStats.st_size);
    parser.PrintCodeToFile();*/
    readStream.close();
}

u8 MMU::Read(u16 address) {
	if (address < 0x8000) {
        if (!IsBootRomEnabled() || (address >= 0x0100 && address <= 0x014F)) {
            // TODO MBC: https://gekkio.fi/files/gb-docs/gbctr.pdf
            return cartridge == nullptr ? 0xFF : cartridge->Read(address);
        } else
            return bootRom[address];
	} else if (address < 0xA000)
		return gpu->Read(address);
	else if (address < 0xC000) // external (cart) ram
		return cartridge == nullptr ? 0xFF : cartridge->Read(address);
	else if (address < 0xE000) {
		if (address < 0xD000)
			return internalRAM[address - 0xC000];
		else
			return internalRAM[address - 0xC000 + bankNIndex * 0x1000];
	} else if (address < 0xFE00) {
		if (address < 0xF000)
			return internalRAM[address - 0xE000];
		else
			return internalRAM[address - 0xE000 + bankNIndex * 0x1000];
	} else if (address < 0xFEA0)
		return oam[address - 0xFE00];
	else if (address < 0xFF00)
		return 0; //TODO validate if 0 or 0xFF
    else {
        IAddressable* addressable = GetAddresableFor(address);
        if (addressable != nullptr)
            return addressable->Read(address);

        if (IsUnusedReg(address))
            return 0xFF;

        if (address < 0xFF80)
            return ioPorts[address - 0xFF00];
        else
            return zeroPageRAM[address - 0xFF80];
    }
}

void MMU::Write(u16 address, u8 value) {
	if (address < 0x8000) {
		if (cartridge != nullptr)
			cartridge->Write(value, address);
	} else if (address < 0xA000)
		gpu->Write(value, address);
	else if (address < 0xC000) {
		if (cartridge != nullptr)
			cartridge->Write(value, address);
	} else if (address < 0xE000) {
		if (address < 0xD000)
			internalRAM[address - 0xC000] = value;
		else
            internalRAM[address - 0xC000 + bankNIndex * 0x1000] = value;
	} else if (address < 0xFE00) {
		if (address < 0xF000)
			internalRAM[address - 0xE000] = value;
		else
            internalRAM[address - 0xE000 + bankNIndex * 0x1000] = value;
	} else if (address < 0xFEA0)
		oam[address - 0xFE00] = value;
	else if (address < 0xFF00) { // unused
		//TODO ignore?
    } else {
        IAddressable* addressable = GetAddresableFor(address);
        if (addressable != nullptr) {
            addressable->Write(value, address);
            return;
        }

		if (address < 0xFF80) {
            if (IsUnusedReg(address))
                return;

            if (address == 0xFF70) {
                u8 bankIndex = value & 0x03;
                bankNIndex = bankIndex == 0 ? 1 : bankIndex;
                // TODO confirm if writing a 0 should be read as 0 or as 1 too
                value |= bankNIndex;
            }
            
            ioPorts[address - 0xFF00] = value;
		} else
            zeroPageRAM[address - 0xFF80] = value;
    }
}

bool MMU::IsUnusedReg(u16 address) {
    switch (address) {
    case 0xFF02:
        return true;
    }
    return false;
}

void MMU::WriteBit(u16 address, u8 bitPosition, bool set) {
	u8 reg = Read(address);
	if (set)
		reg |= (1 << bitPosition);
	else
		reg &= ~(1 << bitPosition);
	Write(address, reg);
}

bool MMU::IsBootRomEnabled() {
	return Read(0xFF50) == 0;
}

void MMU::ResetInterruptFlag(u8 interruptPosition) {
	WriteBit(0xFF0F, interruptPosition, false);
}

void MMU::SetInterruptFlag(u8 interruptPosition) {
	WriteBit(0xFF0F, interruptPosition, true);
}

IAddressable* MMU::GetAddresableFor(u16 address) {
    if (address >= 0xFF30 && address <= 0xFF3F)
        return audio;

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
	case 0xFF68: // CGB
	case 0xFF69:
	case 0xFF6A:
	case 0xFF6B:
	case 0xFF4F:
    case 0xFF4C: // GBC Non-CGB mode
    case 0xFF46: // dma
    case 0xFF51: // CGB
    case 0xFF52:
    case 0xFF53:
    case 0xFF54:
    case 0xFF55:
		return gpu;
	case 0xFF00:
		return joypad;
	case 0xFF04:
	case 0xFF05:
	case 0xFF06:
	case 0xFF07:
		return timer;
    case 0xFF10:
    case 0xFF11:
    case 0xFF12:
    case 0xFF13:
    case 0xFF14:
    case 0xFF16:
    case 0xFF17:
    case 0xFF18:
    case 0xFF19:
    case 0xFF1A:
    case 0xFF1B:
    case 0xFF1C:
    case 0xFF1D:
    case 0xFF1E:
    case 0xFF20:
    case 0xFF21:
    case 0xFF22:
    case 0xFF23:
    case 0xFF24:
    case 0xFF25:
    case 0xFF26:
        return audio;
	case 0xFF01:
	case 0xFF02:
		return serial;
	case 0xFF0F:
	case 0xFFFF:
		return interruptServiceRoutine;
	}

	return nullptr;
}

void MMU::Copy(u16 from, u16 to) {
	Write(to, Read(from));
}

void MMU::Load(std::ifstream& stream) const {
	stream.read((char*)internalRAM, 0x8000);
    stream.read((char*)&bankNIndex, 1);
	stream.read((char*)ioPorts, 128);
	stream.read((char*)oam, 160);
	stream.read((char*)zeroPageRAM, 128);
}

void MMU::Save(std::ofstream& stream) const {
	stream.write((const char*)internalRAM, 0x8000);
    stream.write((const char*)&bankNIndex, 1);
	stream.write((const char*)ioPorts, 128);
	stream.write((const char*)oam, 160);
	stream.write((const char*)zeroPageRAM, 128);
}
