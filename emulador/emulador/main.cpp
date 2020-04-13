#include <iostream>
#include <chrono>
#include <SFML/Audio.hpp>

#include "Types.h"
#include "GameBoy.h"
#include "CustomAudioStream.h"
#include "Roms.h"

#include "GameWindow.h"
#include "TileViewer.h"
#include "TileMapViewer.h"
#include "SpritesViewer.h"
#include "SoundViewer.h"
#include "StateViewer.h"
#include "PalettesViewer.h"

bool isLittleEndian() {
	u8 t8[2] = { 0x1, 0x0 };
	u16* t16 = (u16*)t8;
	bool isLittleEndian = t16[0] == t8[0];
	std::cout << "t8[0] = " << (unsigned int)t8[0] << " t16[0] = " << (unsigned int)t16[0] << " - is " << (isLittleEndian ? "little endian" : "big endian") << std::endl;
	return isLittleEndian;
}

int main(int argc, char *argv[]) {
    Logger logger;

	std::string romPath;
	std::string romDir;
	std::string romName;
    std::string gameName;

	if (argc > 1) {
		romPath = argv[1];
		size_t slashPosition = romPath.find_last_of("/");
		romDir = romPath.substr(0, slashPosition + 1);
		romName = romPath.substr(slashPosition + 1);
		std::cout << "Loading rom from argv[1] = " << romPath << std::endl;
	} else {
		romDir = "D:/Programacion/gb/roms/";
        romName = Games::BOMB_JACK;
		romPath = romDir.append(romName);
		std::cout << "Loading hardcoded rom from = " << romPath << std::endl;
	}

    size_t slashPos = romName.rfind("/");
    slashPos = slashPos == std::string::npos ? 0 : slashPos + 1;
    
    size_t extensionPos = romName.rfind(".");
    gameName = romName.substr(slashPos, extensionPos - slashPos);

    GameBoy gameBoy(romPath);

    sf::Vector2i p(50, 50);
	GameWindow gameWindow(160, 144, "Game", p, gameBoy.gpu, gameName);
    gameBoy.gpu.gameWindow = &gameWindow;

    p.x = 385;
    p.y = 50;
	TileViewer tilesWindow(128, 192, "Tiles", p, gameBoy);
    
    p.x = 1110;
    p.y = 380;
	SpritesViewer spritesWindow(256 + 8, 256 + 16, "Sprites", p, gameBoy);
    
    p.x = 50;
    p.y = 380;
	TileMapViewer tileMap0Window(256, 256, "Tile map 0", p, gameBoy, 0);
    
    p.x = 580;
    p.y = 380;
	TileMapViewer tileMap1Window(256, 256, "Tile map 1", p, gameBoy, 1);
    
    p.x = 50;
    p.y = 380;
    SoundViewer soundWindow(735, 128, "Sound", p, gameBoy.audio);

	StateViewer stateWindow(gameBoy);

	PalettesViewer palettesWindow(gameBoy);

	bool openViewersOnLaunch = false;
	if (openViewersOnLaunch) {
		tilesWindow.Open();
		spritesWindow.Open();
		tileMap0Window.Open();
		tileMap1Window.Open();
		stateWindow.Open();
		palettesWindow.Open();
        gameWindow.GetFocus();
	}

	// Used only when not syncing emulation with audio
	constexpr u16 SamplesSize = 735;
	sf::Int16 samples[SamplesSize] = { 0 };
	u32 sampleIndex = 0;
	sf::SoundBuffer Buffer;
	sf::Sound Sound;

	u16 framesCount = 0;
	auto previousFPSTime = std::chrono::system_clock::now();

    CustomAudioStream audioStream(gameBoy);
    if (gameBoy.syncWithAudio)
        audioStream.play();
	
    u8 framesToLog = 0;
    u16 breakAt = 0x0101;
	while (gameWindow.IsOpen()) {
		if (!gameBoy.syncWithAudio) {
            gameBoy.MainLoop();

            if (gameBoy.sampleGenerated) {
                samples[sampleIndex] = gameBoy.audio.sample;
                sampleIndex++;

                if (sampleIndex == SamplesSize) {
                    Buffer.loadFromSamples(samples, SamplesSize, 1, 44100);
                    Sound.setBuffer(Buffer);
                    Sound.play();
                    sampleIndex = 0;
                }
            }
        }

        /*if (gameBoy.cpu.pc == breakAt) {
            gameBoy.isPaused = true;
            stateWindow.Open();
            tileMap0Window.Open();
            //palettesWindow.Open();
            gameWindow.GetFocus();
        }*/

        if (gameBoy.isPaused || gameBoy.frameFinished) {
            if (framesToLog > 0) {
                framesToLog--;
                if (framesToLog == 0) {
                    gameBoy.cpu.log = false;
                    gameBoy.gpu.log = false;
                    gameBoy.gpu.dma.log = false;
                    gameBoy.cartridge.mbc->log = false;
                    gameBoy.joypad.log = false;
                }
            }

            gameWindow.Update();
            tilesWindow.Update();
            tileMap0Window.Update();
            tileMap1Window.Update();
            spritesWindow.Update();
			stateWindow.Update();
			palettesWindow.Update();

            sf::Event event;
            while (gameWindow.PollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    gameWindow.Close();
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Escape) {
                        gameWindow.Close();
                    }
                    else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num7) {
                        switch (event.key.code) {
                        case sf::Keyboard::Num1: tilesWindow.Toggle(); break;
                        case sf::Keyboard::Num2: tileMap0Window.Toggle(); break;
                        case sf::Keyboard::Num3: tileMap1Window.Toggle(); break;
                        case sf::Keyboard::Num4: spritesWindow.Toggle(); break;
                        case sf::Keyboard::Num5: soundWindow.Toggle(); break;
						case sf::Keyboard::Num6: stateWindow.Toggle(); break;
						case sf::Keyboard::Num7: palettesWindow.Toggle(); break;
                        }
                        gameWindow.GetFocus();
                    }
                    else if (event.key.code >= sf::Keyboard::Num8 && event.key.code <= sf::Keyboard::Num0) {
                        switch (event.key.code) {
                        case sf::Keyboard::Num8: gameBoy.audio.ToggleChannel(1); break;
                        case sf::Keyboard::Num9: gameBoy.audio.ToggleChannel(2); break;
                        case sf::Keyboard::Num0: gameBoy.audio.ToggleChannel(3); break;
                        //case sf::Keyboard::Num0: gameBoy.audio.ToggleChannel(4); break;
                        }
                    }
                    else if (event.key.code == sf::Keyboard::F9) {
						gameBoy.isPaused = !gameBoy.isPaused;
                    }
                    else if (event.key.code == sf::Keyboard::R) {
                        gameBoy.Reset();
                        gameWindow.Clear();
                    }
                    else if (event.key.code == sf::Keyboard::L) {
                        gameBoy.ToggleLogging();
                        //framesToLog = 2;
                    }
					else if (event.key.code == sf::Keyboard::F3) {
						gameBoy.isPaused = false;
						gameBoy.stepsToEmulate = 1;
					}
					else if (event.key.code == sf::Keyboard::W) {
						gameBoy.isPaused = false;
						gameBoy.stepsToEmulate = 10;
					}
					else if (event.key.code == sf::Keyboard::E) {
						gameBoy.isPaused = false;
						gameBoy.stepsToEmulate = 100;
					}
                    else if (event.key.code == sf::Keyboard::C) {
                        gameWindow.TakeScreenshot();
                    }
                }
            }

            while (tilesWindow.PollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    tilesWindow.Close();
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Escape)
                        tilesWindow.Close();
                    else if (event.key.code == sf::Keyboard::PageUp)
                        tilesWindow.NextPalette();
                    else if (event.key.code == sf::Keyboard::PageDown)
                        tilesWindow.PreviousPalette();
                    else if (event.key.code == sf::Keyboard::End)
                        tilesWindow.ToggleBank();
                    else if (event.key.code == sf::Keyboard::Home)
                        tilesWindow.ToggleBgSprite();
                    else if (event.key.code == sf::Keyboard::R) {
                        if (!audioStream.IsRecording()) {
                            soundWindow.CloseStream();
                            audioStream.StartRecording();
                        }
                        else {
                            audioStream.StopRecording();
                            soundWindow.OpenStream();
                        }
                    }
                } else if (event.type == sf::Event::MouseButtonPressed)
                    tilesWindow.OnMouseClicked(event.mouseButton.x, event.mouseButton.y);
            }

            while (soundWindow.PollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    soundWindow.Close();
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Escape)
                        soundWindow.Close();
                    else if (event.key.code == sf::Keyboard::PageUp)
                        soundWindow.NextFrame();
                    else if (event.key.code == sf::Keyboard::PageDown)
                        soundWindow.PreviousFrame();
                    else if (event.key.code == sf::Keyboard::Add)
                        soundWindow.IncrementFramesPerScreen();
                    else if (event.key.code == sf::Keyboard::Subtract)
                        soundWindow.DecrementFramesPerScreen();
                    else if (event.key.code == sf::Keyboard::Y) {
                        if (audioStream.IsRecording())
                            audioStream.StopRecording();
                        soundWindow.OpenStream();
                    }
                }
            }

			while (spritesWindow.PollEvent(event)) {
				if (event.type == sf::Event::Closed)
					spritesWindow.Close();
				else if (event.type == sf::Event::KeyPressed) {
					if (event.key.code == sf::Keyboard::Escape)
						spritesWindow.Close();
					else if (event.key.code == sf::Keyboard::End)
						spritesWindow.ToggleBackground();
				} else if (event.type == sf::Event::MouseButtonPressed)
                    spritesWindow.OnMouseClicked(event.mouseButton.x, event.mouseButton.y);
			}

            while (tileMap0Window.PollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    tileMap0Window.Close();
                else if (event.type == sf::Event::MouseButtonPressed)
                    tileMap0Window.OnMouseClicked(event.mouseButton.x, event.mouseButton.y);
            }

            while (tileMap1Window.PollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    tileMap1Window.Close();
                else if (event.type == sf::Event::MouseButtonPressed)
                    tileMap1Window.OnMouseClicked(event.mouseButton.x, event.mouseButton.y);
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
