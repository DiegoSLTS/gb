#pragma once

#include <SFML\Audio.hpp>
#include <functional>
#include <fstream>

#include "Types.h"
#include "GameBoy.h"

constexpr u16 SamplesSize = 735;
constexpr u8 MaxRecordedSeconds = 5;

class CustomAudioStream : public sf::SoundStream {
public:
    CustomAudioStream(GameBoy& gameBoy);

    void StartRecording();
	void StopRecording();
	bool IsRecording() { return recording; }

protected:
    virtual bool onGetData(Chunk& data);

    virtual void onSeek(sf::Time timeOffset) {}

private:
    GameBoy& gameBoy;

    sf::Int16 samples[SamplesSize] = { 0 };
    u32 sampleIndex = 0;

    u32 recordedFrames = 0;
    bool recording = false;
	std::ofstream OutFile;
};