#pragma once

/*
				GB								GBC
Cpu 			4mhz 							8mhz
Ram 			8k 								32k
Vram 			8k 								16k
Resolution 		160x144 						160x144
Max Tiles 		256 (8x8 px) - 360 onscreen 	512 (8x8 px) - 360 onscreen
Max Sprites 	40 (8x8 px 10 per line) 		40 (8x8 px 10 per line)
Colors 			4 								4 per palette - 8 palletes (0-7) from 32768 colors
Sound chip 		GBZ80 PAPU 						GBZ80 PAPU

DOCS
http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
http://bgb.bircd.org/pandocs.htm
https://rednex.github.io/rgbds/gbz80.7.html

*/

#include "Types.h"
#include "IState.h"
#include "CPUFlags.h"

class MMU;
class InterruptServiceRoutine;

enum CPU8BitReg {
	// this order asumes the system is little endian
	// TODO find solution that works for big endian too
	f = 0,
	a,
	c,
	b,
	e,
	d,
	l,
	h
};

enum CPU16BitReg {
	af = 0,
	bc,
	de,
	hl
};

class CPU : IState {
public:
	CPU();
	virtual ~CPU();
	
	MMU* mmu = nullptr;
	InterruptServiceRoutine* interruptService = nullptr;

	// registers
	u8 registers[8] = { 0 };
	u16 pc = 0, sp = 0;

	u8 lastOpCycles = 0;
	bool isHalted = false;

	u8 Read8BitReg(CPU8BitReg reg) const;
	u16 Read16BitReg(CPU16BitReg reg) const;

	void Write8BitReg(CPU8BitReg reg, u8 value);
	void Write16BitReg(CPU16BitReg reg, u16 value);

	u8 ReadAcc() const;
	void WriteAcc(u8 value);

	u16 ReadHL() const;
	void WriteHL(u16 value);

	void Push16(u16 value);
	u16 Pop16();
	// registers

	// memory
	u8 ReadMemory(u16 address);
	void WriteMemory(u16 address, u8 value);

	u8 ReadAtPC();
	u8 ReadOpCode();
	// memory

	// flags
	u8 ReadFlags() const;
	void WriteFlags(u8 newFlags);
	void ResetFlags();
	void SetFlag(FlagBit flagBit, bool set);
	bool HasFlag(FlagBit flagBit) const;

	void UpdateZeroFlag(u8 value);
	void UpdateHalfCarryFlag(u8 previous, u8 current, bool isAdd);
	void UpdateCarryFlag(u8 previous, u8 current, bool isAdd);

	void UpdateZeroFlag(u16 value);
	void UpdateHalfCarryFlag(u16 previous, u16 current, bool isAdd);
	void UpdateCarryFlag(u16 previous, u16 current, bool isAdd);
	// flags

	// API
	void CallOpCode(u8 opCode);
	void CallCBOpCode(u8 opCode);

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
	//

	// instructions
	void ADCAr8(CPU8BitReg reg);
	void ADCAHL();
	void ADCAn8();
	void ADDAr8(CPU8BitReg reg);
	void ADDAHL();
	void ADDAn8();
	void ANDAr8(CPU8BitReg reg);
	void ANDAHL();
	void ANDAn8();
	void ADDHLr16(CPU16BitReg reg);
	void ADDHLSP();
	void ADDSPe8();
	void BITu3r8(u8 bit, CPU8BitReg reg);
	void BITu3HL(u8 bit);
	void CALLn16();
	void CALLccn16(bool condition);
	void CCF();
	void CPAr8(CPU8BitReg reg);
	void CPAHL();
	void CPAn8();
	void CPL();
	void DAA();
	void DECr8(CPU8BitReg reg);
	void DECHL();
	void DECr16(CPU16BitReg reg);
	void DECSP();
	void DI();
	void EI();
	void HALT();
	void INCr8(CPU8BitReg reg);
	void INCHL();
	void INCr16(CPU16BitReg reg);
	void INCSP();
	void JPHL();
	void JPn16();
	void JPccn16(bool condition);
	void JRe8();
	void JRcce8(bool condition);
	void LDr8r8(CPU8BitReg leftReg, CPU8BitReg rightReg);
	void LDr8n8(CPU8BitReg reg);
	void LDr16n16(CPU16BitReg reg);
	void LDHLr8(CPU8BitReg reg);
	void LDHLn8();
	void LDr8HL(CPU8BitReg reg);
	void LDr16A(CPU16BitReg reg);
	void LDn16A();
	void LDHn8A();
	void LDHCA();
	void LDAr16(CPU16BitReg reg);
	void LDAn16();
	void LDHAn8();
	void LDHAC();
	void LDHLincA();
	void LDHLdecA();
	void LDAHLinc();
	void LDAHLdec();
	void LDSPn16();
	void LDn16SP();
	void LDHLSPe8();
	void LDSPHL();
	void NOP();
	void ORAr8(CPU8BitReg reg);
	void ORAHL();
	void ORAn8();
	void POPr16(CPU16BitReg reg);
	void PUSHr16(CPU16BitReg reg);
	void RESu3r8(u8 bit, CPU8BitReg reg);
	void RESu3HL(u8 bit);
	void RETcc(bool condition);
	void RET();
	void RETI();
	void RLr8(CPU8BitReg reg);
	void RLHL();
	void RLA();
	void RLCr8(CPU8BitReg reg);
	void RLCHL();
	void RLCA();
	void RRr8(CPU8BitReg reg);
	void RRHL();
	void RRA();
	void RRCr8(CPU8BitReg reg);
	void RRCHL();
	void RRCA();
	void RSTvec(u8 vec);
	void SBCAr8(CPU8BitReg reg);
	void SBCAHL();
	void SBCAn8();
	void SCF();
	void SETu3r8(u8 bit, CPU8BitReg reg);
	void SETu3HL(u8 bit);
	void SLAr8(CPU8BitReg reg);
	void SLAHL();
	void SRAr8(CPU8BitReg reg);
	void SRAHL();
	void SRLr8(CPU8BitReg reg);
	void SRLHL();
	void STOP();
	void SUBAr8(CPU8BitReg reg);
	void SUBAHL();
	void SUBAn8();
	void SWAPr8(CPU8BitReg reg);
	void SWAPHL();
	void XORAr8(CPU8BitReg reg);
	void XORAHL();
	void XORAn8();
	// instructions

};
