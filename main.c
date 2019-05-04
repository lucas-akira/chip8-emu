#include "gui.h"

int main(int argc, char *argv[]) {
	
	Chip8 chip8;
	initialize(&chip8);	
	if (argc == 1) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 0;
	}	
 	if (!loadProgram(chip8.memory, argv[1])) {
		return-1;
	}

	int exit_code = 0;
	exit_code = runGUI(&chip8);	
	return exit_code;
}
