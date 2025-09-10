//
// Created by maidy-77 on 10.09.2025.
//

#include <SDL2/SDL.h>
#include "cpu.h"
#include <iostream>

int main() {
    init();
    /* V0 = 10, V1 = 5, V0 -= V1, V2 = 10, V3 = 8, V2 -= V3 RTFM */
    uint8_t program0[] {
        0x60, 0x0A, // LD V0, 10
        0x61, 0x05, // LD V1, 5
        0x80, 0x15, // SUB V0, V1
        0x62, 0x0A, // LD V2, 10
        0x63, 0x08, // LD V3, 8
        0x82, 0x35 // LD V2, V3

    };

    /* put values in registers */
    uint8_t program1[] = {
        0x60, 0x0A, // LD V0, 10
        0x61, 0x14  // LD V1, 20
    };

    /* add up the values in the registers */
    uint8_t program2[] = {
        0x60, 0x05, // LD V0, 5
        0x61, 0x07, // LD V1, 7
        0x80, 0x14  // ADD V0, V1 -> V0 = 12
    };

    /* calculate the values in the registers */
    uint8_t program3[] = {
        0x60, 0x0A, // LD V0, 10
        0x61, 0x03, // LD V1, 3
        0x80, 0x15  // SUB V0, V1 -> V0 = 7
    };

    /* jump to address 0x204 and place the numbers in the registers */
    uint8_t program4[] = {
        0x12, 0x04, // JP 0x204
        0x60, 0x0A, // LD V0, 10
        0x60, 0x14  // LD V0, 20
    };

    /* testing subroutines.
     * jump to subroutine at 0x206, place value in register,
     * then exit subroutine and place other values, then jump to 0x208 */
    uint8_t program5[] = {
        0x22, 0x06, // CALL 0x206
        0x60, 0x0A, // LD V0, 10
        0x12, 0x08, // JP 0x208
        // subroutine at 0x206:
        0x61, 0x14, // LD V1, 20
        0x00, 0xEE  // RET
    };


    // load program1 in memory
    memcpy(&memory[0x200], program0, sizeof(program0));

    // Set program counter to 0x200
    pc = START;

    // Read and run program
    for (int i = 0; i < sizeof(program0)/2; ++i) cycle();

    std::cout << "10 - 5 = " << (int)V[0] << std::endl;
    std::cout << "10 - 8 = " << (int)V[2]<< std::endl;
}

