#pragma once

#include "IAddressable.h"

class InterruptServiceRoutine;

class Joypad : public IAddressable {
public:
    Joypad(InterruptServiceRoutine& interruptService);
    virtual ~Joypad();

	void Update();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

    bool log = false;

private:
	InterruptServiceRoutine& interruptService;

    u8 JOYP = 0xFF; //0xFF00
};
