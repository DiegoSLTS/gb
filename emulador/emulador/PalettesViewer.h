#pragma once

#include "Types.h"
#include "Window.h"

class GameBoy;
class CPU;
class GPU;

class PalettesViewer : public Window {
public:
	PalettesViewer(GameBoy& gameBoy);
	virtual ~PalettesViewer();

	void Update();

private:
	GPU& gpu;
	u8* bgpMemory = nullptr;
	u8* obpMemory = nullptr;

	sf::Font font;

	// cpu
	sf::Text labelBGP;
	sf::Text labelOBP0;
	sf::Text labelOBP1;
	sf::Text labelBG0;
	sf::Text labelBG1;
	sf::Text labelBG2;
	sf::Text labelBG3;
	sf::Text labelBG4;
	sf::Text labelBG5;
	sf::Text labelBG6;
	sf::Text labelBG7;
	sf::Text labelOB0;
	sf::Text labelOB1;
	sf::Text labelOB2;
	sf::Text labelOB3;
	sf::Text labelOB4;
	sf::Text labelOB5;
	sf::Text labelOB6;
	sf::Text labelOB7;

	sf::Text valueBGP;
	sf::Text valueOBP0;
	sf::Text valueOBP1;
	sf::Text valueBG0;
	sf::Text valueBG1;
	sf::Text valueBG2;
	sf::Text valueBG3;
	sf::Text valueBG4;
	sf::Text valueBG5;
	sf::Text valueBG6;
	sf::Text valueBG7;
	sf::Text valueOB0;
	sf::Text valueOB1;
	sf::Text valueOB2;
	sf::Text valueOB3;
	sf::Text valueOB4;
	sf::Text valueOB5;
	sf::Text valueOB6;
	sf::Text valueOB7;

	void SetupLabel(sf::Text& label, const std::string& text, float x, float y);
	void SetupValue(sf::Text& value, float x, float y);

	std::string ToHex(u8 value);
	std::string ToHex(u16 value);

	std::string PaletteToString(u8 index, bool bg);
};
