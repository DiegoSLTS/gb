#pragma once

#include <string>

#include "IAddressable.h"
#include "IState.h"

class Cartridge : public IAddressable, public IState {
public:
    Cartridge(const std::string& romPath);
    virtual ~Cartridge();

	u8 rom[32 * 1024] = { 0 };

	void LoadFile(const std::string& path);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};

