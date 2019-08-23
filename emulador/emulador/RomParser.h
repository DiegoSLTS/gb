#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <deque>

struct Instruction {
	u16 address = 0;
	u8 opCode = 0;
	u8 byte1 = 0;
	u8 byte2 = 0;
	std::string displayText;
};

struct CodeSection {
	u16 addressStart = 0;
	u16 addressEnd = 0;
	std::vector<Instruction> instructions;
};

class RomParser {
public:

	std::vector<CodeSection> sections;
	std::deque<u16> jumpTargets;
	u16 pc = 0;

	void ParseSection(const u8* bytes, u16 from, u16 to);
	Instruction ParseInstruction(const u8* bytes, u16 size);

	bool IsAlreadyParsed(u16 address);

	void ParseBiosROM(const u8* bytes, u16 size);
	void ParseCartridgeROM(const u8* bytes, u16 size);
	void Parse(const u8* bytes, u16 size);

	u8 GetSize(u8 opCode) const;
	std::string GetFormat(u8 opCode, u8 byte1) const;

	void PrintCode();
	void PrintInstruction(const Instruction& instruction);

	void PrintCodeToFile();
	void PrintInstructionToFile(std::ostream& stream, const Instruction& instruction);

	size_t GetInsertIndex(const CodeSection& section) const;
	bool ContainsAnotherSection(const CodeSection& section) const;

	bool AreAllNOP(const u8* bytes, u16 from, u16 to) const;

	std::string ByteToHex(u8 byte);
};