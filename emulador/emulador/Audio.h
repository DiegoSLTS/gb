#pragma once

#include "IAddressable.h"
#include "IState.h"

class Audio : public IAddressable, public IState {
public:
	Audio();
	virtual ~Audio();

	// Sound Channel 1 - Tone & Sweep
	uint8_t NR10 = 0x80; // 0xFF10 Sweep register (R/W)
	uint8_t NR11 = 0; // 0xFF11 Sound length/Wave pattern duty (R/W)
	uint8_t NR12 = 0; // 0xFF12 Volume Envelope (R/W)
	uint8_t NR13 = 0; // 0xFF13 Frequency lo (Write Only)
	uint8_t NR14 = 0; // 0xFF14 Frequency hi (R/W)

	// Sound Channel 2 - Tone
	uint8_t NR21 = 0; // 0xFF16 Sound Length/Wave Pattern Duty (R/W)
	uint8_t NR22 = 0; // 0xFF17 Volume Envelope (R/W)
	uint8_t NR23 = 0; // 0xFF18 Frequency lo data (W)
	uint8_t NR24 = 0; // 0xFF19 Frequency hi data (R/W)

	// Sound Channel 3 - Wave Output
	uint8_t NR30 = 0x7F; // 0xFF1A Sound on/off (R/W)
	uint8_t NR31 = 0; // 0xFF1B Sound Length
	uint8_t NR32 = 0b10011111; // 0xFF1C Select output level (R/W)
	uint8_t NR33 = 0; // 0xFF1D Frequency's lower data (W)
	uint8_t NR34 = 0; // 0xFF1E Frequency's higher data (R/W)
	uint8_t WaveRAM[0x10] = { 0 }; // 0xFF30-0xFF3F Wave Pattern RAM

	// Sound Channel 4 - Noise
	uint8_t NR41 = 0b11000000; // 0xFF20 Sound Length (R/W)
	uint8_t NR42 = 0; // 0xFF21 Volume Envelope (R/W)
	uint8_t NR43 = 0; // 0xFF22 Polynomial Counter (R/W)
	uint8_t NR44 = 0b00111111; // 0xFF23 Counter/consecutive; Inital (R/W)

	// Sound Control Registers
	uint8_t NR50 = 0; // 0xFF24 Channel control / ON-OFF / Volume (R/W)
	uint8_t NR51 = 0; // 0xFF25 Selection of Sound output terminal (R/W)
	uint8_t NR52 = 0b01110000; // 0xFF26 Sound on/off 

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};