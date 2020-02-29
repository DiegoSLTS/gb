#pragma once

#include "Types.h"

class IAddressable {
public:
	virtual u8 Read(u16 address) = 0;
	virtual void Write(u8 value, u16 address) = 0;
};
