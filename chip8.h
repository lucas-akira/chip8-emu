#define WIDTH 64
#define HEIGHT 32

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* short: 2 Bytes */
/* char:  1 Byte  */

typedef struct chip8 {
	unsigned short opcode;		/* Stores the current opcode to be executed */		
	unsigned char memory[4096];	/* 4kiB of memory */
	unsigned char V[16]; 		/* Indexes from 0 to 14 (V0, V1, ..., VE):  General purpose registers; index 15 (VF): carry flag */
	unsigned short index_reg;	/* Index register */
	unsigned short pc;		/* Program Counter */
	unsigned char gfx[WIDTH*HEIGHT];	/* Graphics matrix - Black and white screen of 2048 pixels (64x32) */
	unsigned char update_screen;	/* If this is true (1), update the screen */

	/* Interupts and hardware registers */
	unsigned char delay_timer;
	unsigned char sound_timer;

	/* Stack */
	unsigned short stack[16];
	unsigned short sp;		/* Stack Pointer */

	/* Hexadecimal keypad */
	unsigned char keypad[16];	
} Chip8;

/* Font set - 4px wide and 5px high */
unsigned char chip8_fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
	0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
	0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
  	0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
	0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
 	0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
  	0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
	0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
	0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
	0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
	0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
	0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
	0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
	0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
	0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
	0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

void initialize(Chip8 *chip8);
