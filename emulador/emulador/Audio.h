#pragma once

#include "IAddressable.h"
#include "IState.h"

class Audio : public IAddressable, public IState {
public:
	Audio();
	virtual ~Audio();

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;

	bool Step(u8 cycles);

    void ToggleChannel(u8 channel);

	s16 sample;

private:
	float deltaTimePerEmulatedCycle = 1 / (1.0485f * 1000000);
    float sampleTime = 1 / 44100.0f;
	float elapsedTime = 0.0f;

    struct {
        bool Enabled = false;

        u8 FrequencySweepTime = 0; // NR10 - Bit 6-4 - Sweep Time
        bool FrequencyIncrease = true; // NR10 - Sweep Increase/Decrease - 0: Addition(frequency increases) - 1 : Subtraction(frequency decreases)
        u8 FrequencySweepShifts = 0; // NR10 - Bit 2-0 - Number of sweep shift (n: 0-7)
        float FrequencySweepPeriod = 0.0f;
        bool FrequencySweepEnabled = false;

        float DutyCycle = 0.0f; // NR11 - Bit 7-6 - Wave Pattern Duty (Read/Write)
        float Length = 0.0f; // NR11 - Bit 5-0 - Sound length data (Write Only)

        u8 Volume = 0; // NR12 - Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
        bool VolumeDecrease = false; // NR12 - Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
        u8 EnvelopeSweepCount = 0; // NR12 - Bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.)
        float EnvelopeSweepStep = 0.0f;

        u16 FrequencyX = 0; // NR13 - Lower 8 bits of 11 bit frequency (x). Next 3 bits are in NR14
        u16 FrequencyXShadow = 0;
        float Frequency = 0.0f; // Hz
        float Period = 0.0f;

        bool Initial = false; // NR14 - Bit 7 - Initial(1 = Restart Sound)     (Write Only)
        bool UseLength = false; // NR14 - Bit 6 - Counter / consecutive selection (Read / Write) (1 = Stop output when length in NR11 expires)

        float ElapsedTime = 0.0f;
        float FrequencySweepElapsedTime = 0.0f;
        float EnvelopeSweepElapsedTime = 0.0f;
    } channel1;

    struct {
        bool Enabled = false;

        float DutyCycle = 0.0f; // NR21 - Bit 7-6 - Wave Pattern Duty (Read/Write)
        float Length = 0.0f; // NR21 - Bit 5-0 - Sound length data (Write Only)

        u8 Volume = 0; // NR22 - Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
        bool VolumeDecrease = false; // NR22 - Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
        u8 EnvelopeSweepCount = 0; // NR22 - Bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.)
        float EnvelopeSweepStep = 0.0f;

        u16 FrequencyX = 0; // NR23 - Lower 8 bits of 11 bit frequency (x). Next 3 bits are in NR24
        float Frequency = 0.0f; // Hz
        float Period = 0.0f;

        bool Initial = false; // NR24 - Bit 7 - Initial(1 = Restart Sound)     (Write Only)
        bool UseLength = false; // NR24 - Bit 6 - Counter / consecutive selection (Read / Write) (1 = Stop output when length in NR21 expires)

        float ElapsedTime = 0.0f;
        float EnvelopeSweepElapsedTime = 0.0f;
    } channel2;

    struct {
        bool Enabled = false; // NR30 - Bit 7 - Sound Channel 3 Off  (0=Stop, 1=Playback)  (Read/Write)

        float Length = 0.0f; // NR31 - Bit 7-0 - Sound length (t1: 0 - 255)

        u8 Volume = 0; // NR32 - Bit 6-5 - Select output level (Read/Write)

        u16 FrequencyX = 0; // NR33 - Lower 8 bits of an 11 bit frequency (x). Next 3 bits are in NR34
        float Frequency = 0.0f; // Hz
        float Period = 0.0f;

        bool Initial = false; // NR34 - Bit 7 - Initial(1 = Restart Sound)     (Write Only)
        bool UseLength = false; // NR44 - Bit 6 - Counter / consecutive selection (Read / Write) (1 = Stop output when length in NR31 expires)

        float ElapsedTime = 0.0f;
    } channel3;

    struct {
        bool Enabled = false;

        float Length = 0.0f; // NR41 - Bit 5-0 - Sound length data (t1: 0-63)

        u8 Volume = 0; // NR42 - Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
        bool VolumeDecrease = false; // NR42 - Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
        u8 EnvelopeSweepCount = 0; // NR42 - Bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.)
        float EnvelopeSweepStep = 0.0f;

        u8 ShiftClockFrequency = 0; // NR43 - Bit 7-4 - Shift Clock Frequency (s)
        bool CounterWidth15 = false; // NR43 - Bit 3   - Counter Step/Width (0=15 bits, 1=7 bits)
        u8 DividingRatio = 0; // NR43 - Bit 2-0 - Dividing Ratio of Frequencies (r)
        float Frequency = 0.0f;
        float Period = 0.0f;

        bool Initial = false; // NR44 - Bit 7 - Initial(1 = Restart Sound)     (Write Only)
        bool UseLength = false; // NR44 - Bit 6 - Counter / consecutive selection (Read / Write) (1 = Stop output when length in NR21 expires)

        float ElapsedTime = 0.0f;
        float EnvelopeSweepElapsedTime = 0.0f;
    } channel4;

    u16 lfsr = 0x7FFF;
    u8 lfsrOutput = 0;
    
    void UpdateLFSR(float deltaTime);
    inline void UpdateChannel1(float deltaTime);
    inline void UpdateChannel2(float deltaTime);
    inline void UpdateChannel3(float deltaTime);
    inline void UpdateChannel4(float deltaTime);

	s16 GetSample();
    inline s16 GetChannel1Sample();
    inline s16 GetChannel2Sample();
    inline s16 GetChannel3Sample();
    inline s16 GetChannel4Sample();

    bool channel1UserOn = true;
    bool channel2UserOn = true;
    bool channel3UserOn = true;
    bool channel4UserOn = true;

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
};