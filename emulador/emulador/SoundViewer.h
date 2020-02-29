#pragma once

#include "Types.h"
#include "Window.h"
#include <fstream>

class Audio;

class SoundViewer : public Window {
public:
    SoundViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i Position, Audio& audio);
    virtual ~SoundViewer() override;

    void CloseStream();
    void OpenStream();

    void Update();

    void NextFrame();
    void PreviousFrame();

	void IncrementFramesPerScreen();
	void DecrementFramesPerScreen();

private:
    Audio& audio;

    std::ifstream audioStream;
    u16 frameIndex = 0;
	bool eofReached = false;

	sf::Int16* samples = nullptr;
	u16 framesPerScreen = 10;
};
