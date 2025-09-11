//
// Created by maidy-77 on 10.09.2025.
//

#include <iostream>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL2/SDL.h>
#include <unistd.h>

#include "cpu.h"

// keypad mapping to SDL keycodes
uint8_t keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Path to ROM to be loaded must be given as argument\n";
        exit(1);
    }

    // load ROM into memory
    if (!load_rom(argv[1])) {
        std::cerr << "ROM could not be loaded.\n";
        exit(1);
    }

    // SDL setup
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    const int ht = 320, wt = 640; // scaled display size

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Error in initialising SDL " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    window = SDL_CreateWindow("Chip8 Emulator",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              wt, ht,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error in creating window " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        std::cerr << "Error in initializing rendering " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    SDL_RenderSetLogicalSize(renderer, wt, ht);

    // create texture for 64x32 display
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                64, 32);
    if (!texture) {
        std::cerr << "Error in setting up texture " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    // main emulation loop
    while (true) {
        cycle(); // execute one CPU cycle

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) exit(0);

            // handle key presses
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) exit(0);
                for (int i = 0; i < 16; ++i)
                    if (event.key.keysym.sym == keymap[i])
                        set_keypad(i, 1);
            }

            // handle key releases
            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i)
                    if (event.key.keysym.sym == keymap[i])
                        set_keypad(i, 0);
            }
        }

        // draw display if draw flag is set
        if (get_draw_flag()) {
            set_draw_flag(false);
            uint32_t pixels[32 * 64];
            for (int i = 0; i < 32 * 64; i++)
                pixels[i] = (get_display(i) == 0) ? 0xFF000000 : 0xFFFFFFFF;

            SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        usleep(1000); // small delay for CPU timing
    }

    return 0;
}
