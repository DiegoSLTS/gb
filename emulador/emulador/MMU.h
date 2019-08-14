#pragma once

#include "Types.h"
#include "IState.h"

class IAddressable;

class MMU : public IState {
public:

	/*
	Interrupt Enable Register
	--------------------------- FFFF
	Internal High RAM
	--------------------------- FF80
	Empty but unusable for I/O
	--------------------------- FF4C
	I/O ports
	--------------------------- FF00
	Empty but unusable for I/O
	--------------------------- FEA0 
	Sprite Attrib Memory (OAM)
	--------------------------- FE00
	Echo of 8kB Internal RAM
	--------------------------- E000
	8kB Internal RAM 
	--------------------------- C000
	8kB switchable RAM bank
	--------------------------- A000
	8kB Video RAM
	--------------------------- 8000 --
	16kB switchable ROM bank         |
	--------------------------- 4000  |= 32kB Cartrigb
	16kB ROM bank #0                 |
	--------------------------- 0000 --*/
	u8 zeroPageRAM[0x80] = { 0 };
	u8 ioPorts[0x80] = { 0 };
	u8 oam[0xA0] = { 0 };
	u8 internalRAM[0x2000] = { 0 };
	u8 videoRAM[0x2000] = { 0 };

	IAddressable* cartridge = nullptr;
	IAddressable* joypad = nullptr;
	IAddressable* gpu = nullptr;
	IAddressable* timer = nullptr;
	IAddressable* dma = nullptr;
	IAddressable* serial = nullptr;
	IAddressable* interruptServiceRoutine = nullptr;

	u8 Read(u16 address);
	void Write(u16 address, u8 value);
	void WriteBit(u16 address, u8 bitPosition, bool set);
	void Copy(u16 from, u16 to);

	bool IsBootRomEnabled();

	//TODO move to InterruptServiceRoutine
	void SetInterruptFlag(u8 interruptPosition);
	void ResetInterruptFlag(u8 interruptPosition);

	IAddressable* GetAddresableFor(u16 address);

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};