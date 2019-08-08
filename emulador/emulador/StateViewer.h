#pragma once

#include "Window.h"
#include <SFML\System.hpp>

class CPU;
class GPU;
class MMU;

class StateViewer : public Window {
public:
	StateViewer(CPU* Cpu, GPU* Gpu, MMU* Mmu);
	virtual ~StateViewer();

	CPU* cpu = nullptr;
	GPU* gpu = nullptr;
	MMU* mmu = nullptr;

	void Update();

	sf::Font font;

	sf::Text labelAF;
	sf::Text labelBC;
	sf::Text labelDE;
	sf::Text labelHL;
	sf::Text labelPC;
	sf::Text labelSP;

	sf::Text valueAF;
	sf::Text valueBC;
	sf::Text valueDE;
	sf::Text valueHL;
	sf::Text valuePC;
	sf::Text valueSP;

	void SetupLabel(sf::Text& label, const std::string& text, float x, float y);
	void SetupValue(sf::Text& value, float x, float y);

};