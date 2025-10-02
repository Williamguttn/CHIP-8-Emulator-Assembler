# CHIP-8 Emulator & Assembler
A tool written in C that you can use to run CHIP-8 ROMs, or create your own ROMs using assembly language. [Raylib](https://www.raylib.com/) is used for graphics.
<br/>
![1-Player Pong](image.png)

## Planning
Sound will be implemented in the future. Additionally, improvements to the assembler will be made.

## Notes
CHIP-8 has no real "official" assembly language. However, the "instructions" that are embedded to the assembler are closely tied to the original CHIP-8 opcodes (see [Useful Resources](#useful-resources)). There are example programs in ```/asm``` and ```/games```.

## Usage
You can use ```chip8 -h``` to see all options. Here is a quick summary:
* Run ROM: ```chip8 --rom <file> --run```
* Assemble and run an assembly source: ```chip8 --asm <file> --run```
* Assemble program & create ROM: ```chip8 --asm <file> -o <file>```
* To see register values & memory dumps: ```chip8 --asm/--rom <file> --run --print-regs --debug```
* To speed up execution (for slow programs): ```chip8 --asm/--rom <file> --run --opcodes N```. See help for more.

Additionally, there are options to work directly with CHIP-8 opcodes. See examples in ```/opcodes```. To convert to ROM/execute:
```chip8 --hex <file> [-o <file>/--run]```. Additionally, you can convert assembly programs to hex opcodes: ```chip8 --asm <file> --hexout <file>```

## Building / Running
As of now, I have only tested on Windows. You can change paths to [Raylib](https://www.raylib.com/) in ```build.bat```, and then build from there.

## Useful Resources
* https://en.wikipedia.org/wiki/CHIP-8