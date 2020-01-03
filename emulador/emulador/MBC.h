#pragma once

#include <memory>

#include "IAddressable.h"
#include "IState.h"

struct RomHeader {
	u8 title[16] = { 0 }; // 0x0134 - 0x0143
	u16 newLicenseeCode = 0; // 0x0144 - 0x0145 - Only games newer than SGB
	u8 sgbFlag = 0; // 0x0146 - 0x03: Game supports SGB functions
	u8 cartridgeType = 0; // 0x0147 - MBC + RAM + BATTERY combination used
	u8 romSize = 0; // 0x0148 - "32Kb << romSize"
	u8 ramSize = 0; // 0x0149 - 0, 2KB, 8KB, 32KB
	u8 destinationCode = 0; // 0x014A - 0: Japanese, 1: Non-Japanese
	u8 oldLicenseeCode = 0; // 0x014B - 0x33 means newLicenseCode is used
	u8 maskROMVersionNumber = 0; // 0x14C - 
	u8 headerChecksum = 0; // 0x014D - "x=0:FOR i=0134h TO 014Ch:x=x-MEM[i]-1:NEXT" TODO validate?
	u16 globalChecksum = 0; // 0x014E - 0x014F - big endian?

	// CGB only
	char manufacturerCode[4] = { 0 }; // 0x013F - 0x0142
	bool cgbFlag = false; // 0x0143

	void Print();
};

class MBC : public IAddressable, public IState {
public:
	MBC(const RomHeader& header);
	virtual ~MBC();

	void InitArrays();

	void LoadRom(std::ifstream& readStream);

	virtual void LoadRam(std::ifstream& readStream);
	virtual void SaveRam(std::ofstream& readStream);

	virtual u8 Read(u16 address) = 0;
	virtual void Write(u8 value, u16 address) = 0;

	virtual void Load(std::ifstream& stream) const = 0;
	virtual void Save(std::ofstream& stream) const = 0;

protected:
	std::unique_ptr<u8[]> rom;
	std::unique_ptr<u8[]> ram;

private:
	const RomHeader& header;

	virtual u32 GetRomSize() const;
	virtual u32 GetRamSize() const;
};

class RomOnly : public MBC {
public:
	RomOnly(const RomHeader& header);
	virtual ~RomOnly();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};

class MBC1 : public MBC {
public:
	MBC1(const RomHeader& header);
	virtual ~MBC1();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;

private:
	u8 romBank = 1;
	u8 ramBank = 0;
	unsigned int romBankOffset = 16 * 1024;
	unsigned int ramBankOffset = 0;
	u8 ramEnabled = 0; // 0x00 disabled, 0x0A enabled
	u8 romRamSwitch = 0; // 0x00 rom, 0x01 ram
};

class MBC2 : public MBC {
private:
	// actual ram is 512 * 4, but it's easier to access as bytes
	static const u16 mbc2RamSize = 512 * 8;

public:
	MBC2(const RomHeader& header);
	virtual ~MBC2();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
		
	virtual u32 GetRamSize() const override;

private:
	u8 romBank = 1;
	unsigned int romBankOffset = 16 * 1024;
	u8 ramEnabled = 0; // The least significant bit of the upper address byte must be zero to enable/disable cart RAM
};

class MBC3 : public MBC {
public:
	MBC3(const RomHeader& header);
	virtual ~MBC3();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
	
	virtual void LoadRam(std::ifstream& readStream) override;
	virtual void SaveRam(std::ofstream& readStream) override;

private:
	u8 romBank = 1; // 0x01-0x1F
	u8 ramBank = 0; // 0x00-0x03
	unsigned int romBankOffset = 16 * 1024;
	unsigned int ramBankOffset = 0;
	
	bool ramMapped = true;
	u8 ramTimerEnabled = 0; // 0x00 disabled, 0x0A enabled

	u8 mappedRtcRegister = 0x08; // 0x08-0x0C
	//0 - 0x08: rtcS - seconds, 0-59 (0x00-0x3B)
	//1 - 0x09: rtcM - minutes, 0-59 (0x00-0x3B)
	//2 - 0x0A: rtcH - hours, 0-23 (0x00-0x17)
	//3 - 0x0B: rtcDL - day low bits
	//4 - 0x0C: rtcDH - day high bit, halt flag, carry bit
	u8 rtcRegisters[5] = { 0 };

	bool latch0Wrote = false;
};

class MBC5 : public MBC {
public:
	MBC5(const RomHeader& header);
	virtual ~MBC5();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
	
private:
	u16 romBank = 1;
	u8 ramBank = 0;
	unsigned int romBankOffset = 16 * 1024;
	unsigned int ramBankOffset = 0;
	u8 ramEnabled = 0; // 0x00 disabled, 0x0A enabled
};