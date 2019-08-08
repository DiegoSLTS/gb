#pragma once

#include <cstdint>
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
	uint8_t zeroPageRAM[0x80] = { 0 };
	uint8_t ioPorts[0x80] = { 0 };
	uint8_t oam[0xA0] = { 0 };
	uint8_t internalRAM[0x2000] = { 0 };
	uint8_t videoRAM[0x2000] = { 0 };

	IAddressable* cartridge = nullptr;
	IAddressable* joypad = nullptr;
	IAddressable* gpu = nullptr;
	IAddressable* timer = nullptr;
	IAddressable* dma = nullptr;
	IAddressable* serial = nullptr;
	IAddressable* interruptServiceRoutine = nullptr;

	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t value);
	void WriteBit(uint16_t address, uint8_t bitPosition, bool set);
	void Copy(uint16_t from, uint16_t to);

	bool IsBootRomEnabled();

	//TODO move to InterruptServiceRoutine
	void SetInterruptFlag(uint8_t interruptPosition);
	void ResetInterruptFlag(uint8_t interruptPosition);

	IAddressable* GetAddresableFor(uint16_t address);

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};