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
	unsigned char key_layout; /* 0: QWERTY; 1: AZERTY */
} Chip8;

extern unsigned char chip8_fontset[80];

void initialize(Chip8 *chip8);
int loadProgram(unsigned char memory[4096], char *filename);
void emulateCycle(Chip8 *chip);
