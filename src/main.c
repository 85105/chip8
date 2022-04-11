#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include "SDL2/SDL.h"
#include "chip8.h"
#include "chip8keyboard.h"
#include "chip8screen.h"

// const char keyboard_map[CHIP8_TOTAL_KEYS] = {
//     0x00,
//     0x01,
//     0x02,
//     0x03,
//     0x04,
//     0x05,
//     0x06,
//     0x07,
//     0x08,
//     0x09,
//     0x0A,
//     0x0B,
//     0x0C,
//     0x0D,
//     0x0E,
//     0x0F,
// };

// const char keyboard_map[CHIP8_TOTAL_KEYS] = {
//     SDLK_0,
//     SDLK_1,
//     SDLK_2,
//     SDLK_3,
//     SDLK_4,
//     SDLK_5,
//     SDLK_6,
//     SDLK_7,
//     SDLK_8,
//     SDLK_9,
//     SDLK_a,
//     SDLK_b,
//     SDLK_c,
//     SDLK_d,
//     SDLK_e,
//     SDLK_f,
// };

const char keyboard_map[CHIP8_TOTAL_KEYS] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_w,
    SDLK_q,
    SDLK_3,
    SDLK_s,
    SDLK_a,
    SDLK_d,
    SDLK_e,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("You must provide a file to load\n");
        return -1;
    }

    const char *filename = argv[1];
    FILE *f = fopen(filename, "rb"); // binary
    if (!f)
    {
        printf("Failed to open this file");
        return -1;
    }

    /////////////////////////////////////////////////////////////////////////////
    // need to read the size of file to load in the buffer
    fseek(f, 0, SEEK_END); // to the end of the file
    long size = ftell(f);  // return the size of this file
    fseek(f, 0, SEEK_SET); // back to the begining of the file, because need to start this program.

    char buf[size];
    int res = fread(buf, size, 1, f);
    if (res != 1)
    {
        printf("failed to read this file");
        return -1;
    }
    printf("%s\n", buf);

    struct chip8 chip8;
    chip8_init(&chip8);
    chip8_load(&chip8, buf, size);
    chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

    /////////////////////////////////////////////////////////////////////////////
    // test for opcode
    // chip8_exec(&chip8, 0X00E0);

    // chip8_exec(&chip8, 0X1fda);
    // printf("%x\n", chip8.registers.PC);

    // chip8.registers.PC = 0x00;
    // chip8.registers.V[0] = 0x22;
    // chip8_exec(&chip8, 0x3022);
    // printf("%x\n", chip8.registers.PC);

    // chip8.registers.PC = 0x00;
    // chip8.registers.V[2] = 0x22;
    // chip8.registers.V[3] = 0x22;
    // chip8_exec(&chip8, 0x5230);
    // printf("%x\n", chip8.registers.PC);

    // chip8.registers.V[0] = 200;
    // chip8.registers.V[1] = 50;
    // chip8_exec(&chip8, 0x8014);
    // printf("%x\n", chip8.registers.V[0]);
    // printf("%x\n", chip8.registers.V[0x0f]);

    // chip8.registers.I = 0x00;
    // chip8.registers.V[0] = 10;
    // chip8.registers.V[1] = 10;
    // chip8_exec(&chip8, 0xD015);

    // chip8.registers.V[0] = 0x00;
    // chip8_exec(&chip8, 0xF00A);

    /////////////////////////////////////////////////////////////////////////////
    // test for screen draw sprite
    // chip8_screen_draw_sprite(&chip8.screen, 62, 10, &chip8.memory.memory[0x00], 5);
    // chip8_screen_draw_sprite(&chip8.screen, 32, 60, &chip8.memory.memory[0x05], 5);

    /////////////////////////////////////////////////////////////////////////////
    // test for load to memory
    // chip8_load(&chip8, "Hello", sizeof("Hello"));

    /////////////////////////////////////////////////////////////////////////////
    // test for delay_timer
    // chip8.registers.delay_timer = 255;

    /////////////////////////////////////////////////////////////////////////////
    // test for sound_timer
    // chip8.registers.sound_timer = 30;

    /////////////////////////////////////////////////////////////////////////////
    // test for screen
    // chip8_screen_set(&chip8.screen, 10, 1);

    /////////////////////////////////////////////////////////////////////////////
    // test for memeory
    // chip8.registers.V[0x0f] = 50;
    // chip8_momory_set(&chip8.memory, 0x400, 'z');
    // printf("%c\n", chip8_memory_get(&chip8.memory, 50));

    /////////////////////////////////////////////////////////////////////////////
    // test for stack
    // chip8.registers.SP = 0;
    // chip8_stack_push(&chip8, 0xff);
    // chip8_stack_push(&chip8, 0xaa);
    // printf("%x\n", chip8_stack_pop(&chip8));
    // printf("%x\n", chip8_stack_pop(&chip8));

    /////////////////////////////////////////////////////////////////////////////
    // test for keyboard
    // chip8_keyboard_down(&chip8.keyboard, 0x0f);
    // bool is_down = chip8_keyboard_is_down(&chip8.keyboard, 0x0f);
    // printf("%d\n", (int)is_down);

    /////////////////////////////////////////////////////////////////////////////

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        CHIP8_WIDTH * CHIP8_WINDOW_MULTIPLIER, CHIP8_HEIGHT * CHIP8_WINDOW_MULTIPLIER, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

    // chip8.registers.V[0] = 0x00;
    // chip8_exec(&chip8, 0xF00A);
    // printf("%x\n", chip8.registers.V[0]);
    while (1)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                goto out;
                break;

            case SDL_KEYDOWN:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(&chip8.keyboard, key);
                // printf("key is down %x\n", vkey);
                if (vkey != -1)
                {
                    chip8_keyboard_down(&chip8.keyboard, vkey);
                }
            }
            break;

            case SDL_KEYUP:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(&chip8.keyboard, key);
                if (vkey != -1)
                {
                    chip8_keyboard_up(&chip8.keyboard, vkey);
                }
            }
            break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

        for (int x = 0; x < CHIP8_WIDTH; x++)
        {
            for (int y = 0; y < CHIP8_HEIGHT; y++)
            {
                if (chip8_screen_is_set(&chip8.screen, x, y) == 1)
                {
                    SDL_Rect r;
                    r.x = x * CHIP8_WINDOW_MULTIPLIER;
                    r.y = y * CHIP8_WINDOW_MULTIPLIER;
                    r.w = CHIP8_WINDOW_MULTIPLIER;
                    r.h = CHIP8_WINDOW_MULTIPLIER;
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }

        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        // SDL_RenderClear(renderer);

        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        // SDL_Rect r;
        // r.x = 0;
        // r.y = 0;
        // r.w = 10;
        // r.h = 10;
        // SDL_RenderFillRect(renderer, &r);

        SDL_RenderPresent(renderer);

        if (chip8.registers.delay_timer > 0)
        {
            Sleep(1);
            chip8.registers.delay_timer -= 1;
            // printf("delay");
        }
        if (chip8.registers.sound_timer > 0)
        {
            // no consistent sound
            // Beep(500, 100);
            // chip8.registers.sound_timer -= 1;

            // consistent sound
            Beep(500, 100 * chip8.registers.sound_timer);
            chip8.registers.sound_timer = 0;
        }

        unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC);
        chip8.registers.PC += 2;
        chip8_exec(&chip8, opcode);

        // printf("%x\n", opcode); //print instruction
    }
out:

    SDL_DestroyWindow(window);
    return 0;
}
