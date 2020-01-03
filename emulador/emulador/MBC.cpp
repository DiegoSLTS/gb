#include "MBC.h"

#include <iostream>
#include <ctime>

void RomHeader::Print() {
	std::cout << "title: " << title << std::endl;
	std::cout << "newLicenseeCode: " << (unsigned int)newLicenseeCode << std::endl;
	std::cout << "sgbFlag: " << (unsigned int)sgbFlag << std::endl;
	std::cout << "cartridgeType: " << (unsigned int)cartridgeType << std::endl;
	std::cout << "romSize: " << (unsigned int)romSize << std::endl;
	std::cout << "ramSize: " << (unsigned int)ramSize << std::endl;
	std::cout << "destinationCode: " << (unsigned int)destinationCode << std::endl;
	std::cout << "oldLicenseeCode: " << (unsigned int)oldLicenseeCode << std::endl;
	std::cout << "maskROMVersionNumber: " << (unsigned int)maskROMVersionNumber << std::endl;
	std::cout << "headerChecksum: " << (unsigned int)headerChecksum << std::endl;
	std::cout << "globalChecksum: " << (unsigned int)globalChecksum << std::endl;
}

MBC::MBC(const RomHeader& header) : header(header) {}
MBC::~MBC() {}

void MBC::InitArrays() {
	rom = std::make_unique<u8[]>(GetRomSize());
	ram = std::make_unique<u8[]>(GetRamSize());
}

u32 MBC::GetRomSize() const {
	if (header.romSize < 0x08)
		return (32 * 1024) << header.romSize;
	else {
		switch (header.romSize) {
		case 0x52: return 72 * 16 * 1024; // 72 banks
		case 0x53: return 80 * 16 * 1024;
		case 0x54: return 96 * 16 * 1024;
		}
	}

	std::cout << "ERROR: Invalid romSize value " << (unsigned int)header.romSize << std::endl;
	return 0;
}

u32 MBC::GetRamSize() const {
	switch (header.ramSize) {
	case 0x00: return 0;
	case 0x01: return 2 * 1024;
	case 0x02: return 8 * 1024;
	case 0x03: return 32 * 1024; // 4 * 8KBytes banks
	case 0x04: return 128 * 1024;
	case 0x05: return 64 * 1024;
	}

	std::cout << "ERROR: Invalid ramSize value " << (unsigned int)header.ramSize << std::endl;
	return 0;
}

void MBC::LoadRom(std::ifstream& readStream) {
	readStream.read((char*)rom.get(), GetRomSize());
}

void MBC::LoadRam(std::ifstream& readStream) {
	readStream.read((char*)ram.get(), GetRamSize());
}

void MBC::SaveRam(std::ofstream& writeStream) {
	writeStream.write((const char*)ram.get(), GetRamSize());
}

RomOnly::RomOnly(const RomHeader& header) : MBC(header) {}
RomOnly::~RomOnly() {}

u8 RomOnly::Read(u16 address) {
	if (address <= 0x7FFF)
		return rom[address];

	return 0xFF;
}

void RomOnly::Write(u8 value, u16 address) {}
void RomOnly::Load(std::ifstream& stream) const {}
void RomOnly::Save(std::ofstream& stream) const {}

MBC1::MBC1(const RomHeader& header) : MBC(header) {}
MBC1::~MBC1() {}

u8 MBC1::Read(u16 address) {
	if (address < 0x4000) {
		return rom[address];
	} else if (address < 0x8000) {
		unsigned int mappedAddress = romBankOffset + address - 0x4000;
		return rom[mappedAddress];
	} else if (address >= 0xA000 && address < 0xC000) {
		unsigned int mappedAddress = ramBankOffset + address - 0xA000;
		return ram[mappedAddress];
	}

	return 0xFF;
}

void MBC1::Write(u8 value, u16 address) {
	if (address < 0x2000) {
		ramEnabled = value;
	} else if (address >= 0x2000 && address < 0x4000) {
		u8 lowBits = value & 0b00011111;
		if (lowBits == 0)
			lowBits = 1;
		romBank &= 0b01100000;
		romBank |= lowBits;
	} else if (address >= 0x4000 && address < 0x6000) {
		if (romRamSwitch == 0) {
			u8 highBits = value & 0b01100000;
			romBank &= 0b00011111;
			romBank |= highBits;
		} else if (ramEnabled) {
			ramBank = value & 0x03;
			ramBankOffset = 8 * 1024 * ramBank;
		}
	} else if (address >= 0x6000 && address < 0x8000) {
		romRamSwitch = value;
	} else if (address >= 0xA000 && address < 0xC000 && ramEnabled) {
		unsigned int mappedAddress = ramBankOffset + address - 0xA000;
		ram[mappedAddress] = value;
	}

	if (romRamSwitch == 0)
		romBankOffset = 16 * 1024 * romBank;
}

void MBC1::Load(std::ifstream& stream) const {
	//TODO
}

void MBC1::Save(std::ofstream& stream) const {
	//TODO
}

MBC2::MBC2(const RomHeader& header) : MBC(header) {}
MBC2::~MBC2() {}

u32 MBC2::GetRamSize() const {
	return MBC2::mbc2RamSize;
}

u8 MBC2::Read(u16 address) {
	if (address < 0x4000) {
		return rom[address];
	} else if (address < 0x8000) {
		unsigned int mappedAddress = romBankOffset + address - 0x4000;
		return rom[mappedAddress];
	} else if (address >= 0xA000 && address < 0xA200) {
		unsigned int mappedAddress = address - 0xA000;
		return ram[mappedAddress] & 0x0F;
	}

	return 0xFF;
}

void MBC2::Write(u8 value, u16 address) {
	if (address < 0x2000) {
		if ((address & 0x0100) == 0)
			ramEnabled = value;
	} else if (address >= 0x2000 && address < 0x4000) {
		if ((address & 0x0100) > 0) {
			romBank = value & 0x0F;
			romBankOffset = 16 * 1024 * romBank;
		}
	} else if (address >= 0xA000 && address < 0xA200 && ramEnabled) {
		ram[address - 0xA000] = value;
	}
}

void MBC2::Load(std::ifstream& stream) const {
	//TODO
}

void MBC2::Save(std::ofstream& stream) const {
	//TODO
}

MBC3::MBC3(const RomHeader& header) : MBC(header) {}
MBC3::~MBC3() {}

u8 MBC3::Read(u16 address) {
	if (address < 0x4000) {
		return rom[address];
	} else if (address < 0x8000) {
		unsigned int mappedAddress = romBankOffset + address - 0x4000;
		return rom[mappedAddress];
	} else if (address >= 0xA000 && address < 0xC000) {
		if (ramMapped) {
			unsigned int mappedAddress = ramBankOffset + address - 0xA000;
			return ram[mappedAddress];
		} else {
			return rtcRegisters[mappedRtcRegister - 0x08];
		}
	}

	return 0xFF;
}

void MBC3::Write(u8 value, u16 address) {
	if (address < 0x2000) {
		ramTimerEnabled = value;
	} else if (address >= 0x2000 && address < 0x4000) {
		u8 romBank = value & 0x7F;
		if (romBank == 0)
			romBank = 1;
		romBankOffset = 16 * 1024 * romBank;
	} else if (address >= 0x4000 && address < 0x6000) {
		if (value < 0x07) {
			ramMapped = true;
			ramBank = value;
			ramBankOffset = 8 * 1024 * ramBank;
		} else if (value >= 0x08 && value <= 0x0C) {
			ramMapped = false;
			mappedRtcRegister = value;
		}
	} else if (address >= 0x6000 && address < 0x8000) {
		if (value == 0x01 && latch0Wrote) {
			time_t now = time(NULL);
			struct tm localTime;
			localtime_s(&localTime,&now);
			rtcRegisters[0] = localTime.tm_sec;
			rtcRegisters[1] = localTime.tm_min;
			rtcRegisters[2] = localTime.tm_hour;

			u16 rtcYearDay = rtcRegisters[3] | ((rtcRegisters[4] & 0x01) << 8);
			int nowYearDay = localTime.tm_yday;

			rtcRegisters[3] = nowYearDay < 256 ? nowYearDay : 255;
			u8 rtcDH = rtcRegisters[4] & 0xC0; // keep only bits 7, 6, discard 0
			if (nowYearDay > 255)
				rtcDH |= 0x01;
			if (nowYearDay < rtcYearDay)
				rtcDH |= 0x80;

			rtcRegisters[4] = rtcDH;
			latch0Wrote = false;
		} else if (value == 0x00) {
			latch0Wrote = true;
		}
	} else if (address >= 0xA000 && address < 0xC000 && ramTimerEnabled) {
		if (ramMapped) {
			unsigned int mappedAddress = ramBankOffset + address - 0xA000;
			ram[mappedAddress] = value;
		} else {
			rtcRegisters[mappedRtcRegister - 0x08] = value;
		}
	}
}

void MBC3::LoadRam(std::ifstream& readStream) {
	MBC::LoadRam(readStream);
	// TODO load rtc regs?
}

void MBC3::SaveRam(std::ofstream& writeStream) {
	MBC::SaveRam(writeStream);
	// TODO save rtc regs?
}

void MBC3::Load(std::ifstream& stream) const {
	//TODO
}

void MBC3::Save(std::ofstream& stream) const {
	//TODO
}

MBC5::MBC5(const RomHeader& header) : MBC(header) {}
MBC5::~MBC5() {}

u8 MBC5::Read(u16 address) {
	if (address < 0x4000) {
		return rom[address];
	} else if (address < 0x8000) {
		unsigned int mappedAddress = romBankOffset + address - 0x4000;
		return rom[mappedAddress];
	} else if (address >= 0xA000 && address < 0xC000) {
		unsigned int mappedAddress = ramBankOffset + address - 0xA000;
		return ram[mappedAddress];
	}

	return 0xFF;
}

void MBC5::Write(u8 value, u16 address) {
	if (address < 0x2000) {
		ramEnabled = value;
	} else if (address >= 0x2000 && address < 0x3000) {
		u8 lowBits = value;
		romBank &= 0x0100;
		romBank |= lowBits;
	} else if (address >= 0x3000 && address < 0x4000) {
		u8 highBit = value & 0x01;
		romBank &= 0x00FF;
		romBank |= (highBit << 8);
	} else if (address >= 0x4000 && address < 0x6000) {
		if (ramEnabled) {
			ramBank = value & 0x0F;
			ramBankOffset = 8 * 1024 * ramBank;
		}
	} else if (address >= 0xA000 && address < 0xC000 && ramEnabled) {
		unsigned int mappedAddress = ramBankOffset + address - 0xA000;
		ram[mappedAddress] = value;
	}

	if (address >= 0x2000 && address < 0x4000)
		romBankOffset = 16 * 1024 * romBank;
}

void MBC5::Load(std::ifstream& stream) const {
	//TODO
}

void MBC5::Save(std::ofstream& stream) const {
	//TODO
}