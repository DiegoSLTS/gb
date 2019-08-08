#pragma once

#include "IAddressable.h"

class SerialDataTransfer : public IAddressable {
public:
	uint8_t SB = 0; //0xFF01 Serial transfer data (R/W
	uint8_t SC = 0x7E; //0xFF02  Serial Transfer Control (R/W)

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;
};