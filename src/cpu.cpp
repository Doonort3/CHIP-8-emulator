//
// Created by maidy-77 on 10.09.2025.
//

#include "cpu.h"

#include <cstring>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <SDL2/SDL.h>

// cpu
uint8_t V[16];
uint16_t I, pc, sp; //
uint16_t stack[16];
uint8_t delay_timer, sound_timer;

// memory
uint8_t memory[4096];

// display
int display[64 * 32];

// keypad
int keypad[16];

// flags
bool draw_flag;

// fonts
unsigned char fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

void init() {
    pc = START;
    I = 0;
    sp = 0;
    sound_timer = 0;
    delay_timer = 0;
    draw_flag = false;

    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    memset(memory, 0, sizeof(memory));
    memset(display, 0, sizeof(display));
    memset(keypad, 0, sizeof(keypad));

    for (int i = 0; i < 80; ++i) {
        memory[FONTSET_START + i] = fontset[i];
    }
}

bool load_rom(std::string rom) {
    std::ifstream f(rom, std::ios::binary | std::ios::in);
    if (!f.is_open()) {
        std::cerr << "Failed to open file: " << rom << std::endl;
        return false;
    }

    char c;
    int j = 512;
    for (int i = START; f.get(c); ++i) {
        if (j >= MEMORY_SIZE) {
            std::cerr << "Memory exhausted" << std::endl;
            return false;
        }
        memory[j] = c;
        j++;
    }
    return true;
}

bool get_draw_flag() {
    return draw_flag;
}

void set_draw_flag(bool flag) {
    draw_flag = flag;
}

int get_display(int i) {
    return display[i];
}

void set_display(int index, int value) {
    display[index] = value;
}

void cycle() {
    // get opcode from memory
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

    // decode
    uint16_t nnn = opcode & 0x0FFF; // adress
    uint8_t kk = opcode & 0x00FF; // 8bit
    uint8_t n = opcode & 0x000F; // low nibble
    uint8_t x = (opcode & 0x0F00) >> 8; // X
    uint8_t y = (opcode & 0x00F0) >> 4; // Y
    uint8_t op = (opcode & 0xF000) >> 12; // high nibble

    switch (op) {
        case 0x0: // 0 group
            switch (opcode) {
                case 0x00E0: // CLS
                    memset(display, 0, sizeof(display));
                    draw_flag = true;
                    pc += 2;
                    break;

                case 0x00EE: // RET
                    sp--;
                    pc = stack[sp];
                    break;

                default:
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    break;
            }
            break;

        case 0x1: // JP NNN
            pc = nnn;
            break;
        case 0x2:
            stack[sp++] = pc + 2; // CALL NNN
            pc = nnn;
            break;
        case 0x3: // SE Vx, KK
            if (V[x] == kk) pc += 4;
            else pc += 2;
            break;
        case 0x4: // SNE Vx, KK
            if (V[x] != kk) pc += 4;
            else pc += 2;
            break;
        case 0x5: // SE Vx, Vy
            if (V[x] == V[y]) pc += 4;
            else pc += 2;
            break;
        case 0x6: // LD Vx, KK
            V[x] = kk;
            pc += 2;
            break;
        case 0x7: // ADD Vx, KK
            V[x] = V[x] + kk;
            pc += 2;
            break;

        case 0x8:
            switch (n) {
                case 0x0: // LD Vx, Vy
                    V[x] = V[y];
                    break;
                case 0x1: // OR Vx, Vy
                    V[x] |= V[y];
                    break;
                case 0x2: // AND Vx, Vy
                    V[x] &= V[y];
                    break;
                case 0x3: // XOR Vx, Vy
                    V[x] ^= V[y];
                    break;
                case 0x4: {
                    // ADD Vx, Vy with carry
                    uint16_t sum = V[x] + V[y];
                    V[0xF] = (sum > 255);
                    V[x] = sum & 0xFF;
                    break;
                }
                case 0x5: // SUB Vx, Vy
                    V[0xF] = (V[x] > V[y]);
                    V[x] -= V[y];
                    break;
                case 0x6: // SHR Vx {, Vy}
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    break;
                case 0x7: // SUBN Vx, Vy
                    V[0xF] = (V[y] > V[x]);
                    V[x] = V[y] - V[x];
                    break;
                case 0xE: // SHL Vx {, Vy}
                    V[0xF] = (V[x] & 0x80) >> 7;
                    V[x] <<= 1;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    break;
            }
            pc += 2;
            break;

        case 0x9: // SNE Vx, Vy
            if (V[x] != V[y]) pc += 4;
            else pc += 2;
            break;
        case 0xA: // LD I, NNN
            I = nnn;
            pc += 2;
            break;
        case 0xB: // JP V0, NNN
            pc = nnn + V[0];
            break;
        case 0xC: // RND Vx, KK
            V[x] = (rand() % 256) & kk;
            pc += 2;
            break;
        case 0xD: {
            // DRW Vx, Vy, N
            uint8_t vx = V[(opcode & 0x0F00) >> 8];
            uint8_t vy = V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;
            V[0xF] = 0;

            for (int row = 0; row < height; ++row) {
                uint8_t sptri = memory[I + row];
                for (int col = 0; col < 8; ++col) {
                    if (sptri & (0x80 >> col)) {
                        int dx = (vx + col) % DISPLAY_WIDTH;
                        int dy = (vy + row) % DISPLAY_HEIGHT;
                        int index = dx + dy * DISPLAY_WIDTH;
                        if (display[index] == 1) V[0xF] = 1;
                        display[index] ^= 1;
                    }
                }
            }

            draw_flag = true;
            pc += 2;
            break;
        }
        case 0xE: {
            uint8_t reg = (opcode & 0x0F00) >> 8;
            uint8_t keycode = opcode & 0x00FF;

            switch (keycode) {
                case 0x9E: // SKP Vx
                    pc += 2;
                    if (keypad[V[reg]] != 0) pc += 2;
                    break;
                case 0xA1: // SKNP Vx
                    pc += 2;
                    if (keypad[V[reg]] == 0) pc += 2;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    pc += 2;
                    break;
            }
            break;
        }
        case 0xF:
            uint8_t reg = (opcode & 0x0F00) >> 8;
            uint8_t val = opcode & 0x00FF;

            switch (val) {
                case 0x07: V[reg] = delay_timer;
                    break;
                case 0x0A: {
                    bool pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (keypad[i] != 0) {
                            V[reg] = i;
                            pressed = true;
                            break;
                        }
                    }
                    if (!pressed) return;
                    break;
                }
                case 0x15: delay_timer = V[reg];
                    break;
                case 0x18: sound_timer = V[reg];
                    break;
                case 0x1E: {
                    V[0xF] = (I + V[reg] > 0xFFF) ? 1 : 0;
                    I += V[reg];
                    break;
                }
                case 0x29: I = V[reg] * 5;
                    break;
                case 0x33: {
                    memory[I] = V[reg] / 100;
                    memory[I + 1] = (V[reg] / 10) % 10;
                    memory[I + 2] = V[reg] % 10;
                    break;
                }
                case 0x55:
                    for (int i = 0; i <= reg; i++) memory[I + i] = V[i];
                    I += reg + 1;
                    break;
                case 0x65:
                    for (int i = 0; i <= reg; i++) V[i] = memory[I + i];
                    I += reg + 1;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
            }
            pc += 2;
            break;
            // default ?
    }
    if (delay_timer > 0) delay_timer--;
    if (sound_timer > 0) sound_timer--;
}
