#pragma once

#include <string>
#include <memory>

#include "IAddressable.h"
#include "IState.h"
#include "MBC.h"

class Cartridge : public IAddressable, public IState {
public:
    Cartridge(const std::string& romPath);
	virtual ~Cartridge();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;

	bool IsGBCCartridge() const;

    u8* GetRomPtr() { return mbc->GetRomPtr(); }

    MBC* mbc = nullptr;

private:
	RomHeader header;
	bool hasBattery = false;
	std::string romFullPath;
	std::string romName;

	void LoadFile(const std::string& path);
    void InitMBC();

    // Used when reading an uncompressed file
	void LoadHeader(std::ifstream& readStream);

    // Used when reading a compressed file
    void LoadHeader(const char* fileContent);

	void LoadRam(std::ifstream& readStream);
	void SaveRam(std::ofstream& writeStream);
};
