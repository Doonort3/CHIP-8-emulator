
<img width="881" height="259" alt="изображение" src="https://github.com/user-attachments/assets/dc5d00d8-34d8-4557-9bea-9da5501c306d" />

Simple [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) emulator written in C++ using SDL2 for monochrome graphics.
This emulator is not perfect in terms of code or working, but it can run simple games and programs.

## Features

- Full CHIP-8 CPU [instructions](https://en.wikipedia.org/wiki/CHIP-8#Opcode_table)
- 64x32 pixel display
- Keypad input mapped to PC
- Load ROMs
- Debug output for opcode execution

## Build
Requirements: [CMake](https://cmake.org/download/) (>=3.31), [SDL2](https://wiki.libsdl.org/SDL2/Installation), development libs

```bash
git clone https://github.com/Doonort3/CHIP-8-emulator/
cd CHIP-8-emulator
mkdir build && cd build
cmake ..
make
```
For Windows, replace `usleep` (posix) with `sleep` (chrono) in `main.cpp` (114) or just use `wsl`

## Using
``` bash
./CHIP_8_emulator ../programs/<ROM>
```

## Controls
| CHIP-8 Key | PC Key |
| ---------- | ------ |
| 0          | X      |
| 1          | 1      |
| 2          | 2      |
| 3          | 3      |
| 4          | Q      |
| 5          | W      |
| 6          | E      |
| 7          | A      |
| 8          | S      |
| 9          | D      |
| A          | Z      |
| B          | C      |
| C          | 4      |
| D          | R      |
| E          | F      |
| F          | V      |

 ### Have fun!
**with games from 1970s 
or explore the code, which I have endeavored to write as simply as possible <3**

### References
https://github.com/JamesGriffin/CHIP-8-Emulator (ROMs)

https://github.com/wernsey/chip8 (Assembler)

https://en.wikipedia.org/wiki/CHIP-8 (Opcodes, logic, specs)

https://johnearnest.github.io/chip8Archive/ (ROMs)

