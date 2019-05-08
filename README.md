# chip8-emu
![screenshot1](/imgs/screenshot1.png)
![screenshot2](/imgs/screenshot2.png)

CHIP-8 emulator written in C, using OpenGL.

## Brief description

From Wikipedia: CHIP-8 is an interpreted programming language developed by Joseph Weisbecker, initially used in the mid 1970s to allow easier programming of games. There are a number of classic video games ported to CHIP-8 such as Pong, Space Invaders and Tetris.

It is considered one of the easiest emulation projects to undertake, given its simplicity.

## Input
CHIP-8 uses a hexadecimal keyboard:

| **1** | **2** | **3** | **C** |
|---|---|---|---|
| **4** | **5** | **6** | **D** |
| **7** | **8** | **9** | **E** |
| **A** | **0** | **B** | **F** |

Which this emulator maps to:

#### QWERTY
| **1** | **2** | **3** | **4** |
|---|---|---|---|
| **Q** | **W** | **E** | **R** |
| **A** | **S** | **D** | **F** |
| **Z** | **X** | **C** | **V** |

or

#### AZERTY
| **1** | **2** | **3** | **4** |
|---|---|---|---|
| **A** | **Z** | **E** | **R** |
| **Q** | **S** | **D** | **F** |
| **W** | **X** | **C** | **V** |

Default combination on startup is QWERTY. Press TAB to change between QWERTY and AZERTY, if needed.


## Current state

Right now the emulator can boot some ROMs successfully, like Pong (1 player), Tetris, Tic-tac-toe and Breakout. Some demos and programs don't execute correctly (like the clock by Bill Fisher). 

## TODO:
* ~~Fix freezes on some ROMs;~~ *Done. This was due to a wrong implementation of the 0xFX0A opcode.*
* Clear and organize code; (*Partially done*)
* Implement sound effect (beep) and delay timer;
* ~~Optimize code to run better on higher resolutions.~~ *Done. While maximized some games reach 290 FPS.*
* ~~Make an FPS cap to slow down the programs.~~ *Program runs at 60 FPS with updated graphics driver.*


## Used references
* https://en.wikipedia.org/wiki/CHIP-8
* http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

## Acknowledgements
* [Tiago Toledo Junior](https://github.com/TNanukem), a good friend, he gave the original idea of this project;
* [David Jowett](https://github.com/DavidJowett): The OpenGL implementation is based on his CHIP-8 emulator 
(https://github.com/DavidJowett/chip8-emulator).
