#pragma once

#include <cinttypes>

class IAddressable {
public:
	virtual uint8_t Read(uint16_t address) = 0;
	virtual void Write(uint8_t value, uint16_t address) = 0;
};