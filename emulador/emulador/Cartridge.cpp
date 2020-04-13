#include "Cartridge.h"

#include <iostream>
#include <fstream>

#include "unzip.h"

Cartridge::Cartridge(const std::string& romPath) : romFullPath(romPath) {
    LoadFile(romFullPath);

	if (hasBattery) {
		size_t slashPosition = romFullPath.find_last_of("/");
		romName = romFullPath.substr(slashPosition + 1);
		size_t dotPosition = romName.find_last_of(".");
		std::string savePath = romName.substr(0, dotPosition).append(".sav");

		std::ifstream readStream(savePath, std::ios::in | std::ios::binary);

		if (readStream.good())
			LoadRam(readStream);

		readStream.close();
	}
}

Cartridge::~Cartridge() {
	if (hasBattery) {
		size_t dotPosition = romName.find_last_of(".");
		std::string savePath = romName.substr(0, dotPosition).append(".sav");

		std::ofstream writeStream(savePath, std::ios::binary);

		if (writeStream.good())
			SaveRam(writeStream);

		writeStream.close();
	}

    if (mbc != nullptr)
        delete mbc;
}

void Cartridge::InitMBC() {
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
        std::cout << "ERROR: cartridgeType " << (unsigned int)header.cartridgeType << "not supported" << std::endl;
        return;
    }

    mbc->InitArrays();
}

void Cartridge::LoadHeader(std::ifstream& readStream) {
	readStream.seekg(0x0134);

	// copy all fields from header from stream to header struct (not just the title)
	// IMPORTANT: variables defined in the same order as the header
	readStream.read((char*)header.title, 0x014F - 0x0134 + 1);

	for (u8 index = 0; index < 4; index++)
		header.manufacturerCode[index] = header.title[12 + index];
	header.cgbFlag = (header.title[15] == 0x80) || (header.title[15] == 0xC0);

	header.Print();

	readStream.seekg(0,std::ios_base::beg);
}

void Cartridge::LoadHeader(const char* fileContent) {
    // copy all fields from header from stream to header struct (not just the title)
    // IMPORTANT: variables defined in the same order as the header
    memcpy(header.title, fileContent + 0x0134, 0x014F - 0x0134 + 1);

    for (u8 index = 0; index < 4; index++)
        header.manufacturerCode[index] = header.title[12 + index];
    header.cgbFlag = (header.title[15] == 0x80) || (header.title[15] == 0xC0);

    header.Print();
}

void Cartridge::LoadRam(std::ifstream& readStream) {
	mbc->LoadRam(readStream);
}

void Cartridge::SaveRam(std::ofstream& writeStream) {
	mbc->SaveRam(writeStream);
}

void Cartridge::LoadFile(const std::string& path) {
    HZIP hz = OpenZip(path.c_str(), 0);
    if (hz != NULL) {
        ZIPENTRY ze;
        GetZipItem(hz, -1, &ze);

        bool found = false;
        for (int zi = 0; zi < ze.index; zi++) {
            GetZipItem(hz, zi, &ze);
            if ((strcmp(ze.name + strlen(ze.name) - 3, ".gb") == 0) || strcmp(ze.name + strlen(ze.name) - 4, ".gbc") == 0) {
                found = true;
                break;
            }
        }

        if (found) {
            char* buffer = new char[ze.unc_size];
            UnzipItem(hz, ze.index, buffer, ze.unc_size);

            LoadHeader(buffer);
            InitMBC();
            mbc->LoadRom(buffer);
            delete[] buffer;
        } else
            std::cout << "ERROR: No .gb or .gbc file inside zip file at " << path << std::endl;
        CloseZip(hz);
    } else {
        std::ifstream readStream;
        readStream.open(path, std::ios::in | std::ios::binary);

        if (readStream.fail()) {
            char errorMessage[256];
            strerror_s(errorMessage, 256);
            std::cout << "ERROR: Could not open file " << path << ". " << errorMessage << std::endl;
        }

        LoadHeader(readStream);
        InitMBC();
        mbc->LoadRom(readStream);

        readStream.close();
    }

    hasBattery = header.cartridgeType == 0x03 || header.cartridgeType == 0x06 || header.cartridgeType == 0x09
        || header.cartridgeType == 0x0D || header.cartridgeType == 0x0F || header.cartridgeType == 0x10
        || header.cartridgeType == 0x13 || header.cartridgeType == 0x17 || header.cartridgeType == 0x1B
        || header.cartridgeType == 0x1E || header.cartridgeType == 0xFF;
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

bool Cartridge::IsGBCCartridge() const {
    return header.cgbFlag;
}

