//
// Created by maidy-77 on 10.09.2025.
//

#include "cpu.h"

#include <cstring>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <SDL2/SDL.h>

uint8_t V[16]; // General purpose registers: V0..VF
uint16_t I, pc, sp; // Index register, Program counter, Stack pointer
uint16_t stack[16]; // Call stack: stores return addresses for subroutines
uint8_t delay_timer, sound_timer; // Timers: decrement at 60Hz if non-zero

uint8_t memory[4096]; // Main memory: CHIP-8 has 4KB of addressable memory

int display[64 * 32]; // Display buffer: 64x32 monochrome pixels, 0 or 1
int keypad[16]; // Keypad state: 16 keys (0x0-0xF), each 0=up, 1=down
bool draw_flag; // Draw flag: set when display needs to be updated

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

/*
 * The function is responsible for the initial initialization and zeroing of all values
 */
void init() {
    pc = START; // Set the program counter to address 0x200, since 0x000–0x1FF is reserved for the interpreter/font.
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

/**
 * This function is responsible for loading the ROM into memory.
 * @param rom path to rom
 * @return true succeeded or false failed
 */
bool load_rom(std::string rom) {
    std::ifstream f(rom, std::ios::binary | std::ios::in);
    if (!f.is_open()) {
        return false;
    }
    char c;
    int j = 512;
    for (int i = 0x200; f.get(c); i++) {
        if (j >= 4096) {
            std::cerr << "Too large " << rom << std::endl;
            return false;
        }
        memory[i] = (uint8_t) c;
        j++;
    }
    return true;
}

/* Setters and getters */
bool get_draw_flag() {
    return draw_flag;
}

int get_display(int i) {
    return display[i];
}

void set_display(int index, int value) {
    display[index] = value;
}

void set_keypad(int index, int val) {
    keypad[index] = val;
}

void set_draw_flag(bool flag) {
    draw_flag = flag;
}

/*
 * Main processor cycle
 * Fetching opcode from memory
 * Decoding opcode into parts
 */
void cycle() {
    if (pc >= MEMORY_SIZE - 1) return; // safety check

    // fetch instruction
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    std::cout << "PC: " << std::hex << pc << " OPCODE: " << std::hex << opcode << std::endl;

    // decode
    uint16_t nnn = opcode & 0x0FFF; // address
    uint8_t kk = opcode & 0x00FF;   // 8-bit constant
    uint8_t n = opcode & 0x000F;    // low nibble
    uint8_t x = (opcode & 0x0F00) >> 8; // register X
    uint8_t y = (opcode & 0x00F0) >> 4; // register Y
    uint8_t op = (opcode & 0xF000) >> 12; // high nibble

    switch (op) {
        case 0x0:
            switch (opcode) {
                case 0x00E0: // CLS
                    memset(display, 0, sizeof(display));
                    draw_flag = true;
                    pc += 2;
                    break;
                case 0x00EE: // RET
                    if (sp == 0) { std::cerr << "Stack underflow\n"; return; }
                    sp--;
                    pc = stack[sp];
                    break;
                default:
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    pc += 2;
                    break;
            }
            break;

        case 0x1: // JP NNN
            pc = nnn;
            break;

        case 0x2: // CALL NNN
            stack[sp++] = pc + 2;
            pc = nnn;
            break;

        case 0x3: // SE Vx, KK
            pc += (V[x] == kk) ? 4 : 2;
            break;

        case 0x4: // SNE Vx, KK
            pc += (V[x] != kk) ? 4 : 2;
            break;

        case 0x5: // SE Vx, Vy
            pc += (V[x] == V[y]) ? 4 : 2;
            break;

        case 0x6: // LD Vx, KK
            V[x] = kk;
            pc += 2;
            break;

        case 0x7: // ADD Vx, KK
            V[x] += kk;
            pc += 2;
            break;

        case 0x8: // 0x8XY? operations
            switch (n) {
                case 0x0: V[x] = V[y]; break;      // LD Vx, Vy
                case 0x1: V[x] |= V[y]; break;     // OR
                case 0x2: V[x] &= V[y]; break;     // AND
                case 0x3: V[x] ^= V[y]; break;     // XOR
                case 0x4: {                        // ADD with carry
                    uint16_t sum = V[x] + V[y];
                    V[0xF] = (sum > 255);
                    V[x] = sum & 0xFF;
                    break;
                }
                case 0x5: V[0xF] = (V[x] > V[y]); V[x] -= V[y]; break;              // SUB
                case 0x6: V[0xF] = V[x] & 1; V[x] >>= 1; break;                     // SHR
                case 0x7: V[0xF] = (V[y] > V[x]); V[x] = V[y] - V[x]; break;        // SUBN
                case 0xE: V[0xF] = (V[x] & 0x80) >> 7; V[x] <<= 1; break;           // SHL
                default: std::cerr << "Unknown opcode: " << opcode << std::endl; break;
            }
            pc += 2;
            break;

        case 0x9: pc += (V[x] != V[y]) ? 4 : 2; break;          // SNE Vx, Vy

        case 0xA: I = nnn; pc += 2; break;                      // LD I, NNN
        case 0xB: pc = nnn + V[0]; break;                       // JP V0, NNN
        case 0xC: V[x] = (rand() % 256) & kk; pc += 2; break;   // RND Vx, KK

        case 0xD: { // DRW Vx, Vy, N
            uint8_t vx = V[x], vy = V[y];
            V[0xF] = 0;
            for (int row = 0; row < n; ++row) {
                uint8_t sprite = memory[I + row];
                for (int col = 0; col < 8; ++col) {
                    if (sprite & (0x80 >> col)) {
                        int dx = (vx + col) % DISPLAY_WIDTH;
                        int dy = (vy + row) % DISPLAY_HEIGHT;
                        int idx = dx + dy * DISPLAY_WIDTH;
                        if (display[idx] == 1) V[0xF] = 1;
                        display[idx] ^= 1;
                    }
                }
            }
            draw_flag = true;
            pc += 2;
            break;
        }

        case 0xE: { // key operations
            uint8_t reg = x;
            switch (opcode & 0x00FF) {
                case 0x9E: pc += (keypad[V[reg]]) ? 4 : 2; break;   // SKP
                case 0xA1: pc += (!keypad[V[reg]]) ? 4 : 2; break;  // SKNP
                default: std::cerr << "Unknown opcode: " << opcode << std::endl; pc += 2; break;
            }
            break;
        }

        case 0xF: { // misc operations
            uint8_t reg = x;
            switch (opcode & 0x00FF) {
                case 0x07: V[reg] = delay_timer; pc += 2; break; // LD Vx, DT
                case 0x0A: { // LD Vx, K
                    bool pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (keypad[i]) { V[reg] = i; pressed = true; break; }
                    }
                    if (!pressed) return; // wait for key
                    pc += 2;
                    break;
                }
                case 0x15: delay_timer = V[reg]; pc += 2; break;                        // LD DT, Vx
                case 0x18: sound_timer = V[reg]; pc += 2; break;                        // LD ST, Vx
                case 0x1E: V[0xF] = (I + V[reg] > 0xFFF); I += V[reg]; pc += 2; break;  // ADD I, Vx
                case 0x29: I = V[reg] * 5; pc += 2; break;                              // LD F, Vx
                case 0x33: // LD B, Vx
                    memory[I] = V[reg]/100;
                    memory[I+1] = (V[reg]/10)%10;
                    memory[I+2] = V[reg]%10;
                    pc += 2;
                    break;
                case 0x55: for(int i=0;i<=reg;i++) memory[I+i]=V[i]; pc += 2; break;    // LD [I], V0..Vx
                case 0x65: for(int i=0;i<=reg;i++) V[i]=memory[I+i]; pc += 2; break;    // LD V0..Vx, [I]
                default: std::cerr << "Unknown opcode: " << opcode << std::endl; pc += 2; break;
            }
            break;
        }

        default:
            std::cerr << "Unknown opcode: " << opcode << std::endl;
            pc += 2;
            break;
    }

    if (delay_timer > 0) delay_timer--;
    if (sound_timer > 0) sound_timer--;
}

// sorry guys, I'll die if I comment on everything.




