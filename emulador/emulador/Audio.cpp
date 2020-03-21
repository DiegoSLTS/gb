#include "Audio.h"

Audio::Audio() {}
Audio::~Audio() {}

bool Audio::Step(u8 cycles, bool isDoubleSpeedEnabled) {
    float delta = deltaTimePerEmulatedCycle * cycles;
    if (isDoubleSpeedEnabled)
        delta /= 2;
	elapsedTime += delta;
    
    UpdateChannel1(delta);
    UpdateChannel2(delta);
    UpdateChannel3(delta);
    UpdateChannel4(delta);
    
	if (elapsedTime >= sampleTime) {
		elapsedTime -= sampleTime;
		sample = GetSample();
		return true;
	}
	return false;
}

s16 Audio::GetSample() {
	if ((NR52 & 0x80) == 0)
		return 0;

	s16 mix = 0;
    s16 channel1Sample = channel1UserOn ? GetChannel1Sample() : 0;
    s16 channel2Sample = channel2UserOn ? GetChannel2Sample() : 0;
    s16 channel3Sample = channel3UserOn ? GetChannel3Sample() : 0;
    s16 channel4Sample = channel4UserOn ? GetChannel4Sample() : 0;
	mix = channel1Sample + channel2Sample + channel3Sample + channel4Sample;

	return mix * 500;
}

s16 Audio::GetChannel1Sample() {
    if (!channel1.Enabled)
        return 0;

	return fmod(channel1.ElapsedTime, channel1.Period) / channel1.Period <= channel1.DutyCycle ? channel1.Volume : -channel1.Volume;
}

s16 Audio::GetChannel2Sample() {
    if (!channel2.Enabled)
        return 0;

    return fmod(channel2.ElapsedTime, channel2.Period) / channel2.Period <= channel2.DutyCycle ? channel2.Volume : -channel2.Volume;
}

s16 Audio::GetChannel3Sample() {
    if (!channel3.Enabled || channel3.Volume == 0)
        return 0;

    float temp = fmod(channel3.ElapsedTime, channel3.Period) / channel3.Period;
    temp *= 16;

    float byteNum;
    float reminder = modf(temp, &byteNum);

    u8 waveByte = WaveRAM[(int)byteNum];
	if (byteNum <= 0.5f)
		waveByte >>= 4;
	else
		waveByte &= 0x0F;

	if (channel3.Volume > 1)
		waveByte >>= (channel3.Volume - 1);

    return (s16)waveByte - 8;
}

s16 Audio::GetChannel4Sample() {
    if (!channel4.Enabled)
        return 0;

    return lfsrOutput == 0 ? channel4.Volume : -channel4.Volume;
}

void Audio::UpdateChannel1(float deltaTime) {
    if (channel1.Enabled) {
        channel1.ElapsedTime += deltaTime;

        if (channel1.UseLength && channel1.ElapsedTime > channel1.Length) {
            channel1.Enabled = false;
            return;
        }
        
        if (channel1.FrequencySweepPeriod > 0.0f && channel1.FrequencySweepShifts > 0 && channel1.FrequencySweepEnabled) {
            channel1.FrequencySweepElapsedTime += deltaTime;

            if (channel1.FrequencySweepElapsedTime >= channel1.FrequencySweepPeriod) {
                u16 temp = channel1.FrequencyXShadow >> channel1.FrequencySweepShifts;
                if (channel1.FrequencyIncrease) {
                    channel1.FrequencyXShadow += temp;
                    if (channel1.FrequencyXShadow > 2047) { // overflow
                        channel1.FrequencyXShadow = 2047;
                        channel1.Enabled = false;
                        return;
                    }
                } else {
                    // TODO "Note that sweep shifts are repeatedly performed until the new value becomes either less than 0 (the previous value is then retained) or, when incrementing, if the new frequency value exceeds the maximum frequency (131Khz or 2048 in register value)"
                    // http://belogic.com/gba/channel1.shtml
                    //if (temp <= channel1.FrequencyXShadow)
                    channel1.FrequencyXShadow -= temp;
                    //else
                    //    channel1.FrequencySweepEnabled = false;
                }

                channel1.Frequency = 131072.0f / (2048 - channel1.FrequencyXShadow);
                channel1.Period = 1 / channel1.Frequency;

                channel1.FrequencySweepElapsedTime -= channel1.FrequencySweepPeriod;
            }
        }

        if (channel1.EnvelopeSweepStep > 0.0f) {
            channel1.EnvelopeSweepElapsedTime += deltaTime;
            if (channel1.EnvelopeSweepElapsedTime >= channel1.EnvelopeSweepStep) {
                if (channel1.VolumeDecrease && channel1.Volume > 0)
                    channel1.Volume--;
                else if (!channel1.VolumeDecrease && channel1.Volume < 15)
                    channel1.Volume++;

                channel1.EnvelopeSweepElapsedTime -= channel1.EnvelopeSweepStep;
            }
        }
    }
}

void Audio::UpdateChannel2(float deltaTime) {
    if (channel2.Enabled) {
        channel2.ElapsedTime += deltaTime;

        if (channel2.UseLength && channel2.ElapsedTime > channel2.Length) {
            channel2.Enabled = false;
            return;
        }

        if (channel2.EnvelopeSweepStep > 0.0f) {
            channel2.EnvelopeSweepElapsedTime += deltaTime;
            if (channel2.EnvelopeSweepElapsedTime >= channel2.EnvelopeSweepStep) {
                if (channel2.VolumeDecrease && channel2.Volume > 0)
                    channel2.Volume--;
                else if (!channel2.VolumeDecrease && channel2.Volume < 15)
                    channel2.Volume++;

                channel2.EnvelopeSweepElapsedTime -= channel2.EnvelopeSweepStep;
            }
        }
    }
}

void Audio::UpdateChannel3(float deltaTime) {
    if (channel3.Enabled) {
        channel3.ElapsedTime += deltaTime;

        if (channel3.UseLength && channel3.ElapsedTime > channel3.Length)
            channel3.Enabled = false;
    }
}

void Audio::UpdateChannel4(float deltaTime) {
    if (channel4.Enabled) {
        channel4.ElapsedTime += deltaTime;

        if (channel4.UseLength && channel4.ElapsedTime > channel4.Length) {
            channel4.Enabled = false;
            return;
        }

        UpdateLFSR(deltaTime);

        if (channel4.EnvelopeSweepStep > 0.0f) {
            channel4.EnvelopeSweepElapsedTime += deltaTime;
            if (channel4.EnvelopeSweepElapsedTime >= channel4.EnvelopeSweepStep) {
                if (channel4.VolumeDecrease && channel4.Volume > 0)
                    channel4.Volume--;
                else if (!channel4.VolumeDecrease && channel4.Volume < 15)
                    channel4.Volume++;

                channel4.EnvelopeSweepElapsedTime -= channel4.EnvelopeSweepStep;
            }
        }
    }
}

void Audio::ToggleChannel(u8 channel) {
    switch (channel) {
    case 1: channel1UserOn = !channel1UserOn; break;
    case 2: channel2UserOn = !channel2UserOn; break;
    case 3: channel3UserOn = !channel3UserOn; break;
    case 4: channel4UserOn = !channel4UserOn; break;
    }
}

void Audio::UpdateLFSR(float deltaTime) {
    while (deltaTime > channel4.Period) {
        u8 firstBit = lfsr & 0x0001;
        lfsr >>= 1;
        bool newBit = ((firstBit ^ (lfsr & 0x0001)) != 0);

        if (newBit)
            lfsr |= 0x4000;
        else
            lfsr &= 0xBFFF;

        if (!channel4.CounterWidth15) {
            if (newBit)
                lfsr |= 0x0040;
            else
                lfsr &= 0xFFBF;
        }

        lfsrOutput = firstBit ? 0 : 1;

        deltaTime -= channel4.Period;
    }
}

u8 Audio::Read(u16 address) {
	if (address >= 0xFF30 && address <= 0xFF3F)
		return WaveRAM[address - 0xFF30];

	switch (address) {
	case 0xFF10:
		return NR10 | 0x80;
	case 0xFF11:
		return NR11;
	case 0xFF12:
		return NR12;
	case 0xFF13:
		return NR13;
	case 0xFF14:
		return NR14;
	case 0xFF16:
		return NR21;
	case 0xFF17:
		return NR22;
	case 0xFF18:
		return NR23;
	case 0xFF19:
		return NR24;
	case 0xFF1A:
		return NR30 | 0x7F;
	case 0xFF1B:
		return NR31;
	case 0xFF1C:
		return NR32 | 0x9F;
	case 0xFF1D:
		return NR33;
	case 0xFF1E:
		return NR34;
	case 0xFF20:
		return NR41 | 0xC0;
	case 0xFF21:
		return NR42;
	case 0xFF22:
		return NR43;
	case 0xFF23:
		return NR44 | 0x3F;
	case 0xFF24:
		return NR50;
	case 0xFF25:
		return NR51;
	case 0xFF26:
		return NR52 | 0x70;
	}

	return 0xFF;
}

void Audio::Write(u8 value, u16 address) {
	if (address >= 0xFF30 && address <= 0xFF3F) {
		WaveRAM[address - 0xFF30] = value;
		return;
	}

	switch (address) {
	case 0xFF10:
 		NR10 = value | 0x80; break;
	case 0xFF11:
		NR11 = value; break;
	case 0xFF12:
		NR12 = value; break;
	case 0xFF13:
		NR13 = value; break;
	case 0xFF14:
		NR14 = value;
		channel1.Initial = ((NR14 & 0x80) != 0);
		if (channel1.Initial)
			channel1.Enabled = true;

        if (channel1.Enabled) {
			channel1.FrequencySweepTime = (NR10 >> 4) & 0x07;
			channel1.FrequencyIncrease = (NR10 & 0x08) == 0;
			channel1.FrequencySweepShifts = NR10 & 0x07;
			channel1.FrequencySweepPeriod = channel1.FrequencySweepTime / 128.0f;
			channel1.FrequencySweepElapsedTime = 0.0f;
			channel1.FrequencyXShadow = channel1.FrequencyX;
			channel1.FrequencySweepEnabled = channel1.FrequencySweepTime > 0 && channel1.FrequencySweepShifts > 0;

			switch (NR11 >> 6) {
			case 0x00: channel1.DutyCycle = 0.125f; break;
			case 0x01: channel1.DutyCycle = 0.25f; break;
			case 0x02: channel1.DutyCycle = 0.5f; break;
			case 0x03: channel1.DutyCycle = 0.75f; break;
			}
			channel1.Length = (64 - (NR11 & 0x3F)) / 256.0f;

			channel1.Volume = NR12 >> 4;
			channel1.VolumeDecrease = ((NR12 & 0x08) == 0);
			channel1.EnvelopeSweepCount = NR12 & 0x03;
			channel1.EnvelopeSweepStep = channel1.EnvelopeSweepCount / 64.0f;

			channel1.UseLength = ((NR14 & 0x40) != 0);
			channel1.FrequencyX = ((NR14 & 0x07) << 8) + (NR13 & 0x00FF);
			channel1.Frequency = 131072.0f / (2048 - channel1.FrequencyX);
			channel1.Period = 1 / channel1.Frequency;

            channel1.ElapsedTime = 0.0f;
            channel1.EnvelopeSweepElapsedTime = 0.0f;
            channel1.FrequencySweepElapsedTime = 0.0f;
        }
		break;
	case 0xFF16:
		NR21 = value; break;
	case 0xFF17:
		NR22 = value; break;
	case 0xFF18:
		NR23 = value; break;
	case 0xFF19:
		NR24 = value;
		channel2.Initial = ((NR24 & 0x80) != 0);
		if (channel2.Initial)
			channel2.Enabled = true;

        if (channel2.Enabled) {
			switch (NR21 >> 6) {
			case 0x00: channel2.DutyCycle = 0.125f; break;
			case 0x01: channel2.DutyCycle = 0.25f; break;
			case 0x02: channel2.DutyCycle = 0.5f; break;
			case 0x03: channel2.DutyCycle = 0.75f; break;
			}
			channel2.Length = (64 - (NR21 & 0x3F)) / 256.0f;

			channel2.Volume = NR22 >> 4;
			channel2.VolumeDecrease = ((NR22 & 0x08) == 0);
			channel2.EnvelopeSweepCount = NR22 & 0x03;
			channel2.EnvelopeSweepStep = channel2.EnvelopeSweepCount / 64.0f;

			channel2.FrequencyX = NR23 + (channel2.FrequencyX & 0xFF00);

			channel2.UseLength = ((NR24 & 0x40) != 0);
			channel2.FrequencyX = ((NR24 & 0x07) << 8) + (channel2.FrequencyX & 0x00FF);
			channel2.Frequency = 131072.0f / (2048 - channel2.FrequencyX);
			channel2.Period = 1 / channel2.Frequency;

            channel2.ElapsedTime = 0.0f;
            channel2.EnvelopeSweepElapsedTime = 0.0f;
        }
        break;
	case 0xFF1A:
		NR30 = value | 0x7F; break;
	case 0xFF1B:
		NR31 = value; break;
	case 0xFF1C:
		NR32 = value | 0x9F; break;
	case 0xFF1D:
		NR33 = value; break;
	case 0xFF1E:
		NR34 = value;
        channel3.Initial = ((NR34 & 0x80) != 0);
        
        if (channel3.Initial && ((NR30 & 0x80) != 0))
            channel3.Enabled = true;
        else if ((NR30 & 0x80) == 0)
            channel3.Enabled = false;

        if (channel3.Enabled) {
			channel3.Length = (256 - NR31) / 256.0f;

			channel3.Volume = ((NR32 & 0x60) >> 5);

			channel3.FrequencyX = NR33 + (channel3.FrequencyX & 0xFF00);

			channel3.UseLength = ((NR34 & 0x40) != 0);
			channel3.FrequencyX = ((NR34 & 0x07) << 8) + (channel3.FrequencyX & 0x00FF);
			channel3.Frequency = 65536.0f / (2048 - channel3.FrequencyX);
			channel3.Period = 1 / channel3.Frequency;

            channel3.ElapsedTime = 0.0f;
        }
        break;
	case 0xFF20:
		NR41 = value | 0xC0; break;
	case 0xFF21:
		NR42 = value; break;
	case 0xFF22:
        NR43 = value; break;
	case 0xFF23:
		NR44 = value | 0x3F;
        channel4.Initial = ((NR44 & 0x80) != 0);
        
		if (channel4.Initial)
			channel4.Enabled = true;

        if (channel4.Enabled) {
            channel4.Length = (64 - (NR41 & 0x3F)) / 256.0f;

            channel4.Volume = NR42 >> 4;
            channel4.VolumeDecrease = ((NR42 & 0x08) == 0);
            channel4.EnvelopeSweepCount = NR42 & 0x03;
            channel4.EnvelopeSweepStep = channel4.EnvelopeSweepCount / 64.0f;

            channel4.ShiftClockFrequency = (NR43 >> 4);
            channel4.CounterWidth15 = ((NR43 & 0x08) == 0);
            channel4.DividingRatio = NR43 & 0x07;
            u8 r = channel4.DividingRatio == 0 ? 8 : 16 * channel4.DividingRatio;
            channel4.Frequency = 4194304.0f / (r * (1 << (channel4.ShiftClockFrequency + 1)));
            channel4.Period = 1 / channel4.Frequency;

            channel4.UseLength = ((NR44 & 0x40) != 0);

            channel4.ElapsedTime = 0.0f;
            channel4.EnvelopeSweepElapsedTime = 0.0f;
        }
        break;
	case 0xFF24:
		NR50 = value; break;
	case 0xFF25:
		NR51 = value; break;
	case 0xFF26:
		NR52 = (value & 0xF0) | (NR52 & 0x0F) | 0x70; break;
	}
}

void Audio::Load(std::ifstream& stream) const {
    // TODO load channels' state
	stream.read((char*)&NR10, 37);
}

void Audio::Save(std::ofstream& stream) const {
    // TODO save channels' state
	stream.write((const char*)&NR10, 37);
}