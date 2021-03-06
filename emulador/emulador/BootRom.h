#pragma once

#include <cstdint>

// This bytes are based on the code from the link below
const u8 bootRom[256] = {
	/*0*/	0x31,0xFE,0xFF,0xAF,0x21,0xFF,0x9F,0x32,0xCB,0x7C,0x20,0xFB,0x21,0x26,0xFF,0x0E, /*0*/
	/*1*/	0x11,0x3E,0x80,0x32,0xE2,0x0C,0x3E,0xF3,0xE2,0x32,0x3E,0x77,0x77,0x3E,0xFC,0xE0, /*1*/
	/*2*/	0x47,0x11,0x04,0x01,0x21,0x10,0x80,0x1A,0xCD,0x95,0x00,0xCD,0x96,0x00,0x13,0x7B, /*2*/
	/*3*/	0xFE,0x34,0x20,0xF3,0x11,0xD8,0x00,0x06,0x08,0x1A,0x13,0x22,0x23,0x05,0x20,0xF9, /*3*/
	/*4*/	0x3E,0x19,0xEA,0x10,0x99,0x21,0x2F,0x99,0x0E,0x0C,0x3D,0x28,0x08,0x32,0x0D,0x20, /*4*/
	/*5*/	0xF9,0x2E,0x0F,0x18,0xF3,0x67,0x3E,0x64,0x57,0xE0,0x42,0x3E,0x91,0xE0,0x40,0x04, /*5*/
	/*6*/	0x1E,0x02,0x0E,0x0C,0xF0,0x44,0xFE,0x90,0x20,0xFA,0x0D,0x20,0xF7,0x1D,0x20,0xF2, /*6*/
	/*7*/	0x0E,0x13,0x24,0x7C,0x1E,0x83,0xFE,0x62,0x28,0x06,0x1E,0xC1,0xFE,0x64,0x20,0x06, /*7*/
	/*8*/	0x7B,0xE2,0x0C,0x3E,0x87,0xE2,0xF0,0x42,0x90,0xE0,0x42,0x15,0x20,0xD2,0x05,0x20, /*8*/
	/*9*/	0x4F,0x16,0x20,0x18,0xCB,0x4F,0x06,0x04,0xC5,0xCB,0x11,0x17,0xC1,0xCB,0x11,0x17, /*9*/
	/*A*/	0x05,0x20,0xF5,0x22,0x23,0x22,0x23,0xC9,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B, /*A*/
	/*B*/	0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E, /*B*/
	/*C*/	0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC, /*C*/
	/*D*/	0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E,0x3C,0x42,0xB9,0xA5,0xB9,0xA5,0x42,0x3C, /*D*/
	/*E*/	0x21,0x04,0x01,0x11,0xA8,0x00,0x1A,0x13,0xBE,0x20,0xFE,0x23,0x7D,0xFE,0x34,0x20, /*E*/
	/*F*/	0xF5,0x06,0x19,0x78,0x86,0x23,0x05,0x20,0xFB,0x86,0x20,0xFE,0x3E,0x01,0xE0,0x50  /*F*/
};

/*
	https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM

	reset sp
	0x0000 0x31 0xFE 0xFF 		LD SP, 0xFFFE

	write 0s from 0x9FFF to 0x8000 (video ram) backwards
	0x0003 0xAF 				XOR A 			A = 0
	0x0004 0x21 0xFF 0x9F		LD HL, 0x9FFF
	0x0007 0x32					LDD (HL), A		writes A (0) to address HL, decrements HL
	0x0008 0xCB 0x7C			BIT 7,H 		if (H < 0x80) set Z
	0x000A 0x20 0xFB			JR NZ, 0xFB 	if (!Z) jump -5 (pc is at 0x000C, moves to 0x0007)

	setup sound (http://bgb.bircd.org/pandocs.htm)
	0x000C 0x21 0x26 0xFF 		LD HL, 0xFF26	points HL to main sound register
	0x000F 0x0E 0x11			LD C, 0x11
	0x0011 0x3E 0x80			LD A, 0x80
	0x0013 0x32					LDD (HL), A		turns on sound (bit 7 of A == 1), point HL to FF25
	0x0014 0xE2					LDH (C), A		setup FF11 sound register
	0x0015 0x0C 				INC C
	0x0016 0x3E 0xF3			LD A, 0xF3
	0x0018 0xE2 				LDH (C), A		setup FF12 sound register 
	0x0019 0x32					LDD (HL), A		setup FF25 sound register, point HL to FF24
	0x001A 0x3E 0x77			LD A, 0x77
	0x001C 0x77					LD (HL), A		setup FF24 sound register

	setup background palette
	0x001D 0x3E 0xFC			LD A, 0xFC
	0x001F 0xE0 0x47			LDH (0x47), A

	copy Nintendo logo to VRAM
	0x0021 0x11 0x04 0x01 		LD DE, 0x0104	point DE to Nintendo logo tiles in game ROM
	0x0024 0x21 0x10 0x80 		LD HL, 0x8010	point HL to VRAM
	0x0027 0x1A 				LD A, (DE)
	0x0028 0xCD 0x95 0x00 		CALL 0x0095
	0x002B 0xCD 0x96 0x00 		CALL 0x0096
	0x002E 0x13 				INC DE
	0x002F 0x7B					LD A, E
	0x0030 0xFE 0x34 			CP 0x34
	0x0032 0x20 0xF3 			JR NZ, 0xF3

	copy � char to VRAM
	0x0034 0x11 0xD8 0x00 		LD DE, 0x00D8	point DE to � char in boot rom
	0x0037 0x06 0x08 			LD B, 0x08
	0x0039 0x1A 				LD A, (DE)
	0x003A 0x13 				INC DE
	0x003B 0x22 				LDI (HL), A
	0x003C 0x23 				INC HL
	0x003D 0x05 				DEC B
	0x003E 0x20 0xF9 			JR NZ, 0xF9

	setup tilemap for background
	0x0040 0x3E 0x19 			LD A, 0x19
	0x0042 0xEA 0x10 0x99 		LD (0x9910), A
	0x0045 0x21 0x2F 0x99 		LD HL, 0x992F
	0x0048 0x0E 0x0C 			LD C, 0x0C
	0x004A 0x3D 				DEC A
	0x004B 0x28 0x08 			JR Z, 0x08
	0x004D 0x32 				LDD (HL), A
	0x004E 0x0D 				DEC C
	0x004F 0x20 0xF9 			JR NZ, 0xF9
	0x0051 0x2E 0x0F 			LD L, 0x0F
	0x0053 0x18 0xF3 			JR 0xF3

	setup vertical scroll
	0x0055 0x67 				LD H, A
	0x0056 0x3E 0x64 			LD A, 0x64
	0x0058 0x57 				LD D, A
	0x0059 0xE0 0x42 			LDH (0x42), A // set SCY to 0x64

	0x005B 0x3E 0x91 			LD A, 0x91
	0x005D 0xE0 0x40 			LDH (0x40), A // set LCDC to 0x91 -> 10010001 (LCD On, BG On, BG Character Area 0x8000-0x8FFF
	0x005F 0x04 				INC B
	0x0060 0x1E 0x02 			LD E, 0x02
	0x0062 0x0E 0x0C 			LD C, 0x0C
	0x0064 0xF0 0x44 			LDH A, (0x44) // get LY (current line being drawn)
	0x0066 0xFE 0x90 			CP 0x90			// 0x90 == 144, frame finished, vblank started
	0x0068 0x20 0xFA 			JR NZ, 0xFA
	0x006A 0x0D 				DEC C
	0x006B 0x20 0xF7 			JR NZ, 0xF7
	0x006D 0x1D 				DEC E
	0x006E 0x20 0xF2 			JR NZ, 0xF2
	0x0070 0x0E 0x13 			LD C, 0x13
	0x0072 0x24 				INC H
	0x0073 0x7C 				LD A, H
	0x0074 0x1E 0x83 			LD E, 0x83
	0x0076 0xFE 0x62 			CP 0x62
	0x0078 0x28 0x06 			JR Z, 0x06
	0x007A 0x1E 0xC1 			LD E, 0xC1
	0x007C 0xFE 0x64 			CP 0x64
	0x007E 0x20 0x06 			JR NZ, 0x06
	0x0080 0x7B 				LD A, E
	0x0081 0xE2 				LDH (C), A
	0x0082 0x0C 				INC C
	0x0083 0x3E 0x87 			LD A, 0x87
	0x0085 0xE2 				LDH (C), A
	0x0086 0xF0 0x42 			LDH A, (0x42)
	0x0088 0x90 				SUB A, B
	0x0089 0xE0 0x42 			LDH (0x42), A
	0x008B 0x15 				DEC D
	0x008C 0x20 0xD2 			JR NZ, 0xD2
	0x008E 0x05 				DEC B
	0x008F 0x20 0x4F 			JR NZ, 0x4F
	0x0091 0x16 0x20 			LD D, 0x20
	0x0093 0x18 0xCB 			JR 0xCB
	0x0095 0x4F 				LD C, A
	0x0096 0x06 0x04 			LD B, 0x04
	0x0098 0xC5 				PUSH BC
	0x0099 0xCB 0x11 			RL C
	0x009B 0x17					RL A
	0x009C 0xC1 				POP BC
	0x009D 0xCB 0x11 			RL C
	0x009F 0x17 				RL A
	0x00A0 0x05 				DEC B
	0x00A1 0x20 0xF5 			JR NZ, 0xF5
	0x00A3 0x22 				LDI (HL), A
	0x00A4 0x23 				INC HL
	0x00A5 0x22 				LDI (HL), A
	0x00A6 0x23 				INC HL
	0x00A7 0xC9 				RET

	0x00A8-0x00D7				Nintendo logo
	0x00D8-0x00DF 				� char

	0x00E0 0x21 0x04 0x01 		LD HL, 0x0104
	0x00E3 0x11 0xA8 0x00 		LD DE, 0x00A8
	0x00E6 0x1A 				LD A, (DE)
	0x00E7 0x13 				INC DE
	0x00E8 0xBE					CP (HL)
	0x00E9 0x20 0xFE 			JR NZ, 0xFE
	0x00EB 0x23 				INC HL
	0x00EC 0x7D 				LD A, L
	0x00ED 0xFE 0x34 			CP 0x34
	0x00EF 0x20 0xF5 			JR NZ, 0xF5
	0x00F1 0x06 0x19 			LD B, 0x19
	0x00F3 0x78 				LD A, B
	0x00F4 0x86 				ADD A, (HL)
	0x00F5 0x23 				INC HL
	0x00F6 0x05 				DEC B
	0x00F7 0x20 0xFB			JR NZ, 0xFB
	0x00F9 0x86 				ADD A, (HL)
	0x00FA 0x20 0xFE			JR NZ, 0xFE
	0x00FC 0x3E 0x01 			LD A, n
	0x00FE 0xE0 0x50 			LDH (0x50), A
*/
