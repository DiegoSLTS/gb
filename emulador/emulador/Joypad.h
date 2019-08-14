#pragma once

#include "IAddressable.h"

class Joypad : public IAddressable {
public:
	void Update();

    u8 JOYP = 0xFF; //0xFF00

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;
};