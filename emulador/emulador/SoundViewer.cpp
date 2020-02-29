#include "SoundViewer.h"
#include "Audio.h"

#include <iostream>

SoundViewer::SoundViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i Position, Audio& Audio)
    : Window(Width, Height, Title, Position, false), audio(Audio) {}

SoundViewer::~SoundViewer() {
    CloseStream();
}

void SoundViewer::CloseStream() {
	if (audioStream.is_open())
		audioStream.close();
}

void SoundViewer::OpenStream() {
    frameIndex = 0;
	eofReached = false;
	samples = new sf::Int16[735 * framesPerScreen]{ 0 };
    audioStream.open("samples.bin", std::ios::in | std::ios::binary);
    if (audioStream.fail())
        std::cout << "Error loading samples.bin" << std::endl;
    else
        Update();
}

void SoundViewer::NextFrame() {
    if (!eofReached) {
        frameIndex++;
        Update();
    }
}

void SoundViewer::PreviousFrame() {
    if (frameIndex > 0) {
        frameIndex--;
		eofReached = false;
        Update();
    }
}

void SoundViewer::IncrementFramesPerScreen() {
	framesPerScreen++;
	delete[] samples;
	samples = new sf::Int16[735 * framesPerScreen]{ 0 };
	std::cout << "framesPerScreen " << framesPerScreen << std::endl;
	Update();
}

void SoundViewer::DecrementFramesPerScreen() {
	if (framesPerScreen > 1) {
		framesPerScreen--;
		delete[] samples;
		samples = new sf::Int16[735 * framesPerScreen]{ 0 };
		std::cout << "framesPerScreen " << framesPerScreen << std::endl;
		Update();
	}
}

void SoundViewer::Update() {
    audioStream.seekg(frameIndex * 735 * sizeof(sf::Int16));
    audioStream.read((char*)samples, 735 * framesPerScreen * sizeof(sf::Int16));

	if (audioStream.eof()) {
		memset(samples, 0, 735 * framesPerScreen * 2 - (size_t)audioStream.gcount());
		eofReached = true;
		audioStream.close();
		audioStream.open("samples.bin", std::ios::in | std::ios::binary);
	}

    memset(screenArray, 0xFF, width * height * 4);

    u16 x = 0;
    u16 y = 0;

    for (u16 index = 0; index < width; index++) {
        screenArray[width * height * 4 / 2 + index * 4] = 0x77;
        screenArray[width * height * 4 / 2 + index * 4 + 1] = 0x77;
        screenArray[width * height * 4 / 2 + index * 4 + 2] = 0x77;
        screenArray[width * height * 4 / 2 + index * 4 + 3] = 0xFF;
    }

	s16 lastY = (height >> 1) - samples[0] / 200;
	s16 max = 0;
	s16 min = 0;
    for (u16 index = 0; index < 735; index++) {
		if ((index % (735 / framesPerScreen)) == 0) {
			screenArray[index * 4] = 0xFF;
			screenArray[index * 4 + 1] = 0x00;
			screenArray[index * 4 + 2] = 0x00;
			screenArray[index * 4 + 3] = 0xFF;
		}

		s16 sample = samples[index * framesPerScreen] / 200;
        y = (height >> 1) - sample;

		if (sample > max)
			max = sample;
		if (sample < min)
			min = sample;

		if (y > height)
			continue;

        if (lastY < y) {
            for (s16 i = lastY; lastY < y; lastY++) {
                screenArray[width * lastY * 4 + index * 4] = 0x00;
                screenArray[width * lastY * 4 + index * 4 + 1] = 0x00;
                screenArray[width * lastY * 4 + index * 4 + 2] = 0x00;
                screenArray[width * lastY * 4 + index * 4 + 3] = 0xFF;
            }
        }
        else if (lastY > y) {
            for (s16 i = lastY; lastY > y; lastY--) {
                screenArray[width * lastY * 4 + index * 4] = 0x00;
                screenArray[width * lastY * 4 + index * 4 + 1] = 0x00;
                screenArray[width * lastY * 4 + index * 4 + 2] = 0x00;
                screenArray[width * lastY * 4 + index * 4 + 3] = 0xFF;
            }
        }

        screenArray[width * y * 4 + index * 4] = 0x00;
        screenArray[width * y * 4 + index * 4 + 1] = 0x00;
        screenArray[width * y * 4 + index * 4 + 2] = 0x00;
        screenArray[width * y * 4 + index * 4 + 3] = 0xFF;
    }
    
    screenTexture.update(screenArray);
    screenSprite.setTexture(screenTexture, true);

    renderWindow->clear();
    renderWindow->draw(screenSprite);
    renderWindow->display();

    float frameTime = 1 / 60.0f;
    std::cout << "Showing samples for frame " << frameIndex << ". From " << frameTime * frameIndex << " to " <<  frameTime * (frameIndex + framesPerScreen) << std::endl;
	std::cout << "Min " << min << " - Max " << max << std::endl;
}