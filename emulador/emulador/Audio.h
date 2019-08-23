#pragma once

#include "IAddressable.h"
#include "IState.h"

class Audio : public IAddressable, public IState {
public:
	Audio();
	virtual ~Audio();

	// Sound Channel 1 - Tone & Sweep
	u8 NR10 = 0x80;	// 0xFF10 Sweep register (R/W)
	u8 NR11 = 0;	// 0xFF11 Sound length/Wave pattern duty (R/W)
	u8 NR12 = 0;	// 0xFF12 Volume Envelope (R/W)
	u8 NR13 = 0;	// 0xFF13 Frequency lo (Write Only)
	u8 NR14 = 0;	// 0xFF14 Frequency hi (R/W)

	// Sound Channel 2 - Tone
	u8 NR21 = 0;	// 0xFF16 Sound Length/Wave Pattern Duty (R/W)
	u8 NR22 = 0;	// 0xFF17 Volume Envelope (R/W)
	u8 NR23 = 0;	// 0xFF18 Frequency lo data (W)
	u8 NR24 = 0;	// 0xFF19 Frequency hi data (R/W)

	// Sound Channel 3 - Wave Output
	u8 NR30 = 0x7F;				// 0xFF1A Sound on/off (R/W)
	u8 NR31 = 0;				// 0xFF1B Sound Length
	u8 NR32 = 0b10011111;		// 0xFF1C Select output level (R/W)
	u8 NR33 = 0;				// 0xFF1D Frequency's lower data (W)
	u8 NR34 = 0;				// 0xFF1E Frequency's higher data (R/W)
	u8 WaveRAM[0x10] = { 0 };	// 0xFF30-0xFF3F Wave Pattern RAM

	// Sound Channel 4 - Noise
	u8 NR41 = 0b11000000;	// 0xFF20 Sound Length (R/W)
	u8 NR42 = 0;			// 0xFF21 Volume Envelope (R/W)
	u8 NR43 = 0;			// 0xFF22 Polynomial Counter (R/W)
	u8 NR44 = 0b00111111;	// 0xFF23 Counter/consecutive; Inital (R/W)

	// Sound Control Registers
	u8 NR50 = 0;			// 0xFF24 Channel control / ON-OFF / Volume (R/W)
	u8 NR51 = 0;			// 0xFF25 Selection of Sound output terminal (R/W)
	u8 NR52 = 0b01110000;	// 0xFF26 Sound on/off 

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};