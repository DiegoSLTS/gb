#pragma once

#include <fstream>

class IState {
public:
	virtual void Load(std::ifstream& stream) const = 0;
	virtual void Save(std::ofstream& stream) const = 0;
};
