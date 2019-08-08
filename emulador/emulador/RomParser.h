#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <deque>

struct Instruction {
	uint16_t address = 0;
	uint8_t opCode = 0;
	uint8_t byte1 = 0;
	uint8_t byte2 = 0;
	std::string displayText;
};

struct CodeSection {
	uint16_t addressStart = 0;
	uint16_t addressEnd = 0;
	std::vector<Instruction> instructions;
};

class RomParser {
public:

	std::vector<CodeSection> sections;
	std::deque<uint16_t> jumpTargets;
	uint16_t pc = 0;

	void ParseSection(uint8_t* bytes, uint16_t from, uint16_t to);
	Instruction ParseInstruction(uint8_t* bytes, uint16_t size);

	bool IsAlreadyParsed(uint16_t address);

	void ParseBiosROM(uint8_t* bytes, uint16_t size);
	void ParseCartridgeROM(uint8_t* bytes, uint16_t size);
	
	uint8_t GetSize(uint8_t opCode) const;
	std::string GetFormat(uint8_t opCode, uint8_t byte1) const;

	void PrintCode();
	void PrintInstruction(const Instruction& instruction);

	void PrintCodeToFile();
	void PrintInstructionToFile(std::ostream& stream, const Instruction& instruction);

	size_t GetInsertIndex(const CodeSection& section) const;
	bool ContainsAnotherSection(const CodeSection& section) const;

	bool AreAllNOP(uint8_t* bytes, uint16_t from, uint16_t to) const;

	std::string ByteToHex(uint8_t byte);
};