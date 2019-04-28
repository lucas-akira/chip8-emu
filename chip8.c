#include "chip8.h"

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

void initialize(Chip8 *chip) {
	chip->pc = 0x200;	/* Program is loaded in address 0x200 */
	chip->opcode = 0;	/* Reset current opcode */
	chip->index_reg = 0;	/* Reset index register */
	chip->sp = 0;		/* Reset stack pointer */
	
	srand(time(NULL));
	
	/* Clear display */
	unsigned int i = 0;
	for (i = 0; i < WIDTH*HEIGHT; i++) {
		chip->gfx[i] = 0;
	}
	
	/* Clear stack and registers */
	for (i = 0; i < 16; i++) {
		chip->stack[i] = 0;
		chip->V[i] = 0;
	}

	/* Clear memory */
	for (i = 0; i < 4096; i++) {
		chip->memory[i] = 0;
	}
	
	/* Load fontset */
	for (i = 0; i < 80; i++) {
		chip->memory[i] = chip8_fontset[i];
	}

	/* Reset timers */
	chip->delay_timer = 0xFF;
	chip->sound_timer = 0xFF;
}

void loadProgram(unsigned char memory[4096], char *filename) {
	
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Program %s not found!\n", filename);
		return;
	}
	unsigned int i = 0;
	unsigned char current_byte = 0;
	
	/* Program is loaded starting at address 0x200 (512 in decimal) */
	while (fread(&current_byte, sizeof(char), 1, fp) && i < 3584) {
		memory[i + 0x200] = current_byte;
		i++;
	}

	fclose(fp);
	fp = NULL;
	printf("Program loaded into memory\n");
}

void emulateCycle(Chip8 *chip) {
	unsigned short pc = chip->pc; 
	unsigned short opcode = chip->opcode;
	
	unsigned char *V = chip->V;	
	unsigned char *memory = chip->memory;	
	
	/* Fetch opcode */
	opcode = memory[pc] << 8 | memory[pc + 1];

	unsigned char x = ( (opcode & 0x0F00) >> 8 );
	unsigned char y = ( (opcode & 0x00F0) >> 4 );
	unsigned int loop = 0;
	/* Decode opcode of the general form 0xZNNN - except when noted otherwise */
        switch (opcode & 0xF000) {

		case 0x0000: /* 3 possibilities */
			switch (opcode & 0x00FF) {
				case 0x00E0: /* Clear the screen (0x00E0) */
					for (loop = 0; loop < WIDTH * HEIGHT; loop++) {
						chip->gfx[loop] = 0;
					}
					chip->update_screen = 1;
					pc += 2;
					break;

				case 0x00EE: /* Return from a subroutine */
					if (chip->sp == 0) {
						printf("Stack is empty!\n");
					} else {
						(chip->sp)--;
						pc = chip->stack[chip->sp];
					}
					pc += 2;
					break;

				default: /* Call RCA 1802 program at address NNN*/ 
					printf("Calling RCA 1802 program at 0x%x\n", opcode & 0x0FFF);
					pc += 2;
			}
			break;

		case 0x1000: /* goto NNN */
			pc = opcode & 0x0FFF;
			break;

		case 0x2000: /* Call subroutine at NNN */
			chip->stack[chip->sp] = pc;
			(chip->sp)++;
			pc = opcode & 0x0FFF;
			break;

		case 0x3000: /* 0x3XNN -- if (VX == NN): skip next instruction */
			if (V[x] == (opcode & 0x00FF) ) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;

		case 0x4000: /* 0x4XNN -- if (VX != NN): skip next instruction */
			if (V[x] != (opcode & 0x00FF) ) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;

		case 0x5000: /* 0x5XY0 -- if (VX == VY): skip next instruction */
			
			if ( (opcode & 0x000F) == 0 ) {
				if (V[x] == V[y] ) {
					pc += 4;
				} else {
					pc += 2;
				}
			} else {
				printf("Unknown opcode: 0x%x\n", opcode);
			}
			break;

		case 0x6000: /* 0x6XNN -- VX = NN */
			V[x] = (opcode & 0x00FF);
			pc += 2;
			break;

		case 0x7000: /* 0x7XNN -- VX += NN  (Carry flag is not changed) */
			V[x] += (opcode & 0x00FF);
			pc += 2;
			break;

		case 0x8000: /* 9 possibilities of the form 0x8XYZ - where Z defines the different cases */
			switch (opcode & 0x000F) {
				case 0x0000: /* VX = VY */
					V[x] = V[y];
					pc += 2;
					break;

				case 0x0001: /* VX = VX | VY */
					V[x] |= V[y];
					pc += 2;
					break;

				case 0x0002: /* VX = VX & VY */
					V[x] &= V[y];
					pc += 2;
					break;

				case 0x0003: /* VX = VX ^ VY */
					V[x] ^= V[y];
					pc += 2;
					break;

				case 0x0004: /* VX += VY (VF is set to 1 when there is a carry, set to 0 otherwise) */
					if (V[y] > 0xFF - V[x]) {
						V[0xF] = 1;
					} else {
						V[0xF] = 0;
					}
					V[x] += V[y];
					pc += 2;
					break;

				case 0x0005: /* VX -= VY (VF is set to 0 when there is a borrow, set to 1 otherwise) */	
					if (V[x] > V[y]) {
						V[0xF] = 1;
					} else {
						V[0xF] = 0;
					}
					V[x] -= V[y];
					pc += 2;
					break;

				case 0x0006: /* VX = VX >> 1 and store the least significant bit of VX in VF */
					V[0xF] = V[x] & 0x01;
					V[x] >>= 1;
					pc += 2;
					break;

				case 0x0007: /* VX = VY - VX (VF is set to 0 when there is a borrow, set to 1 otherwise) */
					if (V[y] > V[x]) {
						V[0xF] = 1;
					} else {
						V[0xF] = 0;
					}
					V[x] = V[y] - V[x];
					pc += 2;
					break;

				case 0x000E: /* VX = VX << 1 and store the least significant bit of VX in VF */
					V[0xF] = V[x] & 0x01;
					V[x] <<= 1;
					pc += 2;
					break;
				
				default:
					printf("Unknown opcode: 0x%x\n", opcode);
			}
			break;

		case 0x9000: /* 0x9XY0 - if (V[x] != V[y]): skip next instruction */
			if ( (opcode & 0x000F) == 0 ) {
				if (V[x] != V[y] ) {
					pc += 4;
				} else {
					pc += 2;
				}
			} else {
				printf("Unknown opcode: 0x%x\n", opcode);
			}
			break;

		case 0xA000: /* index_reg = NNN -- Set index_reg to address NNN */
			chip->index_reg = (opcode & 0x0FFF);
			pc += 2;
			break;

		case 0xB000: /* pc = V0 + NNN */
			pc = V[0] + (opcode & 0x0FFF);
			break;

		case 0xC000: /* 0xCXNN -- VX = rand() & NN */
			V[x] = (rand() % 256) & (opcode & 0x00FF);
			pc += 2;
			break;

		case 0xD000: /* 0xDXYN -- draw(VX, VY, N): draw a sprite at (VX, VY) with 8px wide and Npx tall; each row of 8px is read as bit-coded starting from memory location index_reg */
		{	
			unsigned char height = (opcode & 0x000F);
			unsigned char sprite_row;
			unsigned char xline = 0, yline = 0;
			V[0xF] = 0;
			for (yline = 0; yline < height; yline++) { /* For each sprite row */

				sprite_row = memory[chip->index_reg + yline]; /* Get sprite row */
				for (xline = 0; xline < 8; xline++) { /* For each pixel in the row */

					if ( (sprite_row & (0x80 >> xline)) != 0 ) { /* Check if the current evaluated pixel is set to 1 */
						unsigned char x_pos = (V[x] + xline) % WIDTH;
						unsigned char y_pos = (V[y] + yline) % HEIGHT;
						if (chip->gfx[x_pos + (y_pos * WIDTH)] == 1 ) /* Check if the pixel on display is set to 1 */
							V[0xF] = 1; /* Pixel collision occured */
						chip->gfx[x_pos + (y_pos * WIDTH)] ^= 1;
					}
				}
			}
			chip->update_screen = 1;
			pc += 2;
			break;
		}
		case 0xE000: /* 0xEXZZ -- where ZZ is either 0x9E or 0xA1 */
			if ( (opcode & 0x00FF) == 0x009E ) { /* Skip the next instruction if the key stored in VX is pressed */
				if (chip->keypad[V[x]] == 1) {
					pc += 4;
				} else {
					pc+= 2;
				}
			} else if  ( (opcode & 0x00FF) == 0x00A1 ) { /* Skip the next instruction if the key stored in VX is not pressed */
				if (chip->keypad[V[x]] == 0) {
					pc += 4;
				} else {
					pc += 2;
				}
			} else {
				printf("Unknown opcode: 0x%x\n", opcode);
			}
			break;
		
		case 0xF000: /* 9 options of the form 0xFXZZ -- where ZZ defines the different cases */
			switch (opcode & 0x00FF) {
				case 0x0007: /* Set VX to the value of delay_timer */
					V[x] = chip->delay_timer;
					pc += 2;
					break;

				case 0x000A: /* Wait for a key press, then store its value in VX */
					scanf( "%c" , &(V[x]) );
					pc += 2;
					break;

				case 0x0015: /* Set delay_timer to VX */
					chip->delay_timer = V[x];
					pc += 2;
					break;
				
				case 0x0018: /* Set sound_timer to VX */
					chip->sound_timer = V[x];
					pc += 2;
					break;

				case 0x001E: /* index_reg += VX */
					chip->index_reg += V[x];
					pc += 2;
					break;

				case 0x0029: /* Set index_reg to the location of the sprite for the character in VX */
					chip->index_reg = (V[x] * 5) % 80;
					pc += 2;
					break;

				case 0x0033: /* Store BCD representation of VX in memory locations index_reg, index_reg+1 and index_reg+2 */
					memory[chip->index_reg] = V[x] / 100;
					memory[chip->index_reg+1] = (V[x] / 10) % 10;
					memory[chip->index_reg+2] = (V[x] % 100) % 10;
					pc += 2;
					break;

				case 0x0055: /* Store registers V0 through VX in memory starting at location index_reg */
					for (loop = 0; loop <= x; loop++) {
						memory[chip->index_reg + loop] = V[loop];
					}
					pc += 2;
					break;

				case 0x0065: /* Fill V0 to VX with values from memory starting at addr index_reg */
					for (loop = 0; loop <= x; loop++) {
						V[loop] = memory[chip->index_reg + loop];
					}
					pc += 2;
					break;
			}
			break;
	

		default:
			printf("Unknown opcode: 0x%x\n", opcode);

       	}
	
	/* Update timers */
	if (chip->delay_timer > 0) {
		--(chip->delay_timer);
	}

	if (chip->sound_timer > 0) {
		if (chip->sound_timer == 1) {
			printf("BEEP\n");
		}
		--(chip->sound_timer);
	}

	/* Update struct variable */
	chip->pc = pc;
}

