//
// Created by maidy-77 on 10.09.2025.
//

#ifndef CHIP_8_EMULATOR_CPU_H
#define CHIP_8_EMULATOR_CPU_H

#include <cstdint>
#include <string>

constexpr int MEMORY_SIZE = 4096;
constexpr int REGISTER_SIZE = 16;
constexpr int STACK_SIZE = 16;
constexpr int DISPLAY_WIDTH = 64;
constexpr int DISPLAY_HEIGHT = 32;
constexpr int START = 0x200;
constexpr int FONTSET_START = 0x50;

extern uint8_t V[16];
extern uint16_t I, pc, sp;
extern uint16_t stack[16];
extern uint8_t delay_timer, sound_timer;
extern uint8_t memory[4096];
extern int display[64 * 32];
extern int keypad[16];
extern bool draw_flag;
extern unsigned char fontset[80];

constexpr int PIXEL_SIZE = 10;

void init();

bool load_rom(std::string rom);

void cycle();

bool get_draw_flag();

void set_draw_flag(bool flag);

int get_display(int);

void set_keypad(int, int);


#endif //CHIP_8_EMULATOR_CPU_H
