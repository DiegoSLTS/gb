#pragma once

#include <cstdint>
#include <string>

#include "IAddressable.h"
#include "IState.h"

class Cartridge : public IAddressable, public IState {
public:
	uint8_t rom[32 * 1024] = { 0 };

	void LoadFile(const std::string& path);

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};

