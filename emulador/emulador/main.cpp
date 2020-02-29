#include <fstream>
#include <iostream>
#include <bitset>
#include <chrono>
#include <SFML/Audio.hpp>

#include "Types.h"
#include "GameBoy.h"

#include "Window.h"
#include "GameWindow.h"
#include "TileViewer.h"
#include "TileMapViewer.h"
#include "SpritesViewer.h"
#include "SoundViewer.h"
#include "StateViewer.h"

#include "Roms.h"

#include "CustomAudioStream.h"

void printCPUFlags(const CPU& cpu) {
	std::bitset<8> flags(cpu.Read8BitReg(CPU8BitReg::f));
	std::cout << "flags: 0b" << flags << std::endl;
}

bool isLittleEndian() {
	u8 t8[2] = { 0x1, 0x0 };
	u16* t16 = (u16*)t8;
	bool isLittleEndian = t16[0] == t8[0];
	std::cout << "t8[0] = " << (unsigned int)t8[0] << " t16[0] = " << (unsigned int)t16[0] << " - is " << (isLittleEndian ? "little endian" : "big endian") << std::endl;
	return isLittleEndian;
}

int main(int argc, char *argv[]) {
	std::string romPath;
	std::string romDir;
	std::string romName;

	if (argc > 1) {
		//TODO validate path
		romPath = argv[1];
		size_t slashPosition = romPath.find_last_of("/");
		romDir = romPath.substr(0, slashPosition + 1);
		romName = romPath.substr(slashPosition + 1);
		std::cout << "Loading rom from argv[1] = " << romPath << std::endl;
	} else {
		romDir = "D:/Programacion/gb/roms/";
		romName = Games::LEGEND_OF_ZELDA_LINKS_AWAKENING;
		romPath = romDir.append(romName);
		std::cout << "Loading hardcoded rom from = " << romPath << std::endl;
	}

    GameBoy gameBoy(romPath);
    
    sf::Vector2i p(50, 50);
	GameWindow gameWindow(160, 144, "Game", p, gameBoy.gpu);
	
    p.x = 385;
    p.y = 50;
	TileViewer tilesWindow(168, 144, "Tiles", p, gameBoy.mmu, 0x8000);

    p.x = 1110;
    p.y = 380;
	SpritesViewer spritesWindow(256 + 8, 256 + 16, "Sprites", p, gameBoy.mmu);
	
    p.x = 50;
    p.y = 380;
	TileMapViewer tileMap0Window(256, 256, "Tile map 0", p, gameBoy.mmu, 0x9800);

    p.x = 580;
    p.y = 380;
	TileMapViewer tileMap1Window(256, 256, "Tile map 1", p, gameBoy.mmu, 0x9C00);

    p.x = 50;
    p.y = 380;
    SoundViewer soundWindow(735, 128, "Sound", p, gameBoy.audio);

    CustomAudioStream audioStream(gameBoy);
    if (gameBoy.syncWithAudio)
        audioStream.play();

    u16 framesCount = 0;
    auto previousFPSTime = std::chrono::system_clock::now();

    // Used only when not syncing emulation with audio
    constexpr u16 Samples_Size = 735;
    sf::Int16 samples[Samples_Size] = { 0 };
    u32 sampleIndex = 0;
    sf::SoundBuffer Buffer;
    sf::Sound Sound;

	while (gameWindow.IsOpen()) {
        if (!gameBoy.syncWithAudio) {
            gameBoy.MainLoop();

            if (gameBoy.sampleGenerated) {
                samples[sampleIndex] = gameBoy.audio.sample;
                sampleIndex++;

                if (sampleIndex == Samples_Size) {
                    Buffer.loadFromSamples(samples, Samples_Size, 1, 44100);
                    Sound.setBuffer(Buffer);
                    Sound.play();
                    sampleIndex = 0;
                }
            }
        }

        if (gameBoy.frameFinished) {
            gameWindow.Update();
            tilesWindow.Update();
            tileMap0Window.Update();
            tileMap1Window.Update();
            spritesWindow.Update();

            sf::Event event;
            while (gameWindow.PollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    gameWindow.Close();
                else if (event.type == sf::Event::KeyPressed) {
					if (event.key.code == sf::Keyboard::Escape) {
						gameWindow.Close();
					} else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num5) {
						switch (event.key.code) {
						case sf::Keyboard::Num1: tilesWindow.Toggle(); break;
						case sf::Keyboard::Num2: tileMap0Window.Toggle(); break;
						case sf::Keyboard::Num3: tileMap1Window.Toggle(); break;
						case sf::Keyboard::Num4: spritesWindow.Toggle(); break;
                        case sf::Keyboard::Num5: soundWindow.Toggle(); break;
						}
						gameWindow.GetFocus();
					} else if (event.key.code >= sf::Keyboard::Num6 && event.key.code <= sf::Keyboard::Num9) {
						switch (event.key.code) {
						case sf::Keyboard::Num6: gameBoy.audio.ToggleChannel(1); break;
						case sf::Keyboard::Num7: gameBoy.audio.ToggleChannel(2); break;
						case sf::Keyboard::Num8: gameBoy.audio.ToggleChannel(3); break;
						case sf::Keyboard::Num9: gameBoy.audio.ToggleChannel(4); break;
						}
					} else if (event.key.code == sf::Keyboard::PageUp)
                        soundWindow.NextFrame();
					else if (event.key.code == sf::Keyboard::PageDown)
                        soundWindow.PreviousFrame();
					else if (event.key.code == sf::Keyboard::Add)
                        soundWindow.IncrementFramesPerScreen();
					else if (event.key.code == sf::Keyboard::Subtract)
                        soundWindow.DecrementFramesPerScreen();
					else if (event.key.code == sf::Keyboard::R) {
						if (!audioStream.IsRecording()) {
                            soundWindow.CloseStream();
							audioStream.StartRecording();
						} else {
							audioStream.StopRecording();
                            soundWindow.OpenStream();
						}
					} else if (event.key.code == sf::Keyboard::Y) {
						if (audioStream.IsRecording())
							audioStream.StopRecording();
                        soundWindow.OpenStream();
					}
                }
            }

            framesCount++;
            auto currentTime = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - previousFPSTime).count() >= 1) {
				gameWindow.SetTitle("Game - FPS: " + std::to_string(framesCount));
                previousFPSTime = currentTime;
                framesCount = 0;
            }
        }
	}

    audioStream.stop();

	return 0;
}
