#include "CustomAudioStream.h"
#include <iostream>


CustomAudioStream::CustomAudioStream(GameBoy& gameBoy) : gameBoy(gameBoy) {
    initialize(1, 44100);
}

bool CustomAudioStream::onGetData(Chunk& data) {
    while (sampleIndex < SamplesSize) {
        gameBoy.MainLoop();
        if (gameBoy.sampleGenerated) {
            samples[sampleIndex] = gameBoy.audio.sample;
            sampleIndex++;
        }
    }

    data.samples = samples;
    data.sampleCount = SamplesSize;

    sampleIndex = 0;

    if (recording) {
		OutFile.write((char*)samples, SamplesSize * sizeof(s16));
		recordedFrames++;
		if (recordedFrames >= 60 * MaxRecordedSeconds)
			StopRecording();
    }
    return true;
}

void CustomAudioStream::StartRecording() {
	if (recording)
		return;

    recording = true;
    recordedFrames = 0;
	OutFile.open("samples.bin", std::ios::out | std::ios::binary);
}

void CustomAudioStream::StopRecording() {
	if (!recording)
		return;

	OutFile.close();
	std::cout << "Recorded " << recordedFrames / 60.0f << " seconds of samples" << std::endl;
	recording = false;
}