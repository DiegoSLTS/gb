#pragma once

#include "Types.h"
#include "Window.h"

class GameBoy;
class CPU;
class GPU;

class StateViewer : public Window {
public:
	StateViewer(GameBoy& gameBoy);
	virtual ~StateViewer();

	void Update();

private:
	CPU& cpu;
	GPU& gpu;

	sf::Font font;

	// cpu
	sf::Text labelPC;
	sf::Text labelSP;

	sf::Text valueAF;
	sf::Text valueBC;
	sf::Text valueDE;
	sf::Text valueHL;
	sf::Text valuePC;
	sf::Text valueSP;

	// gpu
	sf::Text labelLCDC;
	sf::Text labelLCDStat;
	sf::Text labelLY;
	sf::Text labelLYC;
	sf::Text labelSCX;
	sf::Text labelSCY;
	sf::Text labelWX;
	sf::Text labelWY;
	sf::Text labelBGPI;
	sf::Text labelOBPI;
	sf::Text labelBGP;
	sf::Text labelOBP0;
	sf::Text labelOBP1;

	sf::Text valueLCDC;
	sf::Text valueLCDStat;
	sf::Text valueLY;
	sf::Text valueLYC;
	sf::Text valueSCX;
	sf::Text valueSCY;
	sf::Text valueWX;
	sf::Text valueWY;
	sf::Text valueBGPI;
	sf::Text valueOBPI;
	sf::Text valueBGP;
	sf::Text valueOBP0;
	sf::Text valueOBP1;

	void SetupLabel(sf::Text& label, const std::string& text, float x, float y);
	void SetupValue(sf::Text& value, float x, float y);

	std::string ToHex(u8 value);
	std::string ToHex(u16 value);
};
