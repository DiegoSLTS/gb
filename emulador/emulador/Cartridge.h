#pragma once

#include <string>

#include "IAddressable.h"
#include "IState.h"
#include "MBC.h"

class Cartridge : public IAddressable, public IState {
public:
    Cartridge(const std::string& romPath);
    virtual ~Cartridge();

	RomHeader header;
	MBC* mbc = nullptr;
	bool hasBattery = false;

	void LoadFile(const std::string& path);
	void LoadHeader(std::ifstream& readStream);
	void LoadRom(std::ifstream& readStream);

	void LoadRam(std::ifstream& readStream);
	void SaveRam(std::ofstream& writeStream);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};

