#pragma once

#include "IAddressable.h"

class Joypad : public IAddressable {
public:
	void Update();

	uint8_t JOYP = 0xFF; //0xFF00

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;
};