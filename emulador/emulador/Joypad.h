#pragma once

#include "IAddressable.h"

class MMU;

class Joypad : public IAddressable {
public:
    Joypad(MMU& mmu);
    virtual ~Joypad();

	void Update();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

private:
    MMU& mmu;

    u8 JOYP = 0xFF; //0xFF00
};
