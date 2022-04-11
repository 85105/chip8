#include "chip8.h"
#include <memory.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "SDL2/SDL.h"
const char chip8_default_character_set[] = {
    0xf0,
    0x90,
    0x90,
    0x90,
    0xf0,
    0x20,
    0x60,
    0x20,
    0x20,
    0x70,
    0xf0,
    0x10,
    0xf0,
    0x80,
    0xf0,
    0xf0,
    0x10,
    0xf0,
    0x10,
    0xf0,
    0x90,
    0x90,
    0xf0,
    0x10,
    0x10,
    0xf0,
    0x80,
    0xf0,
    0x10,
    0xf0,
    0xf0,
    0x80,
    0xf0,
    0x90,
    0xf0,
    0xf0,
    0x10,
    0x20,
    0x40,
    0x40,
    0xf0,
    0x90,
    0xf0,
    0x90,
    0xf0,
    0xf0,
    0x90,
    0xf0,
    0x10,
    0xf0,
    0xf0,
    0x90,
    0xf0,
    0x90,
    0x90,
    0xe0,
    0x90,
    0xe0,
    0x90,
    0xe0,
    0xf0,
    0x80,
    0x80,
    0x80,
    0xf0,
    0xe0,
    0x90,
    0x90,
    0x90,
    0xe0,
    0xf0,
    0x80,
    0xf0,
    0x80,
    0xf0,
    0xf0,
    0x80,
    0xf0,
    0x80,
    0x80,
};

void chip8_init(struct chip8 *chip8)
{
    memset(chip8, 0, sizeof(struct chip8));
    memcpy(&chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

void chip8_load(struct chip8 *chip8, const char *buf, size_t size)
{
    assert(size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
    memcpy(&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static char chip8_wait_for_key_press(struct chip8 *chip8)
{
    SDL_Event event;
    while (SDL_WaitEvent(&event))
    {
        if (event.type != SDL_KEYDOWN)
            continue;

        char c = event.key.keysym.sym;
        char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);
        if (chip8_key != -1)
        {
            return chip8_key;
        }
    }

    return -1;
}
static void chip8_exec_extended_F(struct chip8 *chip8, unsigned short opcode)
{
    unsigned short x = (opcode >> 8) & 0x000f;
    switch (opcode & 0x00ff)
    {
    case 0x07: // Fx07 - LD Vx, DT //Set Vx = delay timer value.
        chip8->registers.V[x] = chip8->registers.delay_timer;
        break;
    case 0x0A: // Fx0A - LD Vx, K //Wait for a key press, store the value of the key in Vx.
    {
        char pressed_key = chip8_wait_for_key_press(chip8);
        chip8->registers.V[x] = pressed_key;
    }
    break;
    case 0x15:
        chip8->registers.delay_timer = chip8->registers.V[x];
        break;
    case 0x18:
        chip8->registers.sound_timer = chip8->registers.V[x];
        break;
    case 0x1E:
        chip8->registers.I += chip8->registers.V[x];
        break;
    case 0x29:
        chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
        break;
    case 0x33:
    {
        unsigned char hundreds = chip8->registers.V[x] / 100;
        unsigned char tens = chip8->registers.V[x] / 10 % 10;
        unsigned char units = chip8->registers.V[x] % 10;
        chip8_memory_set(&chip8->memory, chip8->registers.I, hundreds);
        chip8_memory_set(&chip8->memory, chip8->registers.I + 1, tens);
        chip8_memory_set(&chip8->memory, chip8->registers.I + 2, units);
    }

    break;
    case 0x55:
    {
        for (int i = 0; i <= x; i++)
        {
            chip8_memory_set(&chip8->memory, chip8->registers.I + i, chip8->registers.V[i]);
        }
    }
    break;
    case 0x65:
    {
        for (int i = 0; i <= x; i++)
        {
            chip8->registers.V[i] = chip8_memory_get(&chip8->memory, chip8->registers.I + i);
        }
    }
    break;
    }
}

static void chip8_exec_extended_eight(struct chip8 *chip8, unsigned short opcode)
{
    unsigned short x = (opcode >> 8) & 0x000f;
    unsigned short y = (opcode >> 4) & 0x000f;
    unsigned short tmp = 0;
    switch (opcode & 0x000f)
    {
    case 0x00: // 8xy0 - LD Vx, Vy  //Set Vx = Vy.
        chip8->registers.V[x] = chip8->registers.V[y];
        break;
    case 0x01: // 8xy1 - OR Vx, Vy //Set Vx = Vx OR Vy.
        chip8->registers.V[x] = chip8->registers.V[y] | chip8->registers.V[x];
        break;
    case 0x02: // 8xy2 - AND Vx, Vy //Set Vx = Vx AND Vy.
        chip8->registers.V[x] = chip8->registers.V[y] & chip8->registers.V[x];
        break;
    case 0x03: // 8xy3 - XOR Vx, Vy //Set Vx = Vx XOR Vy..
        chip8->registers.V[x] = chip8->registers.V[y] ^ chip8->registers.V[x];
        break;
    case 0x04: // 8xy4 - ADD Vx, Vy //Set Vx = Vx + Vy, set VF = carry.
        tmp = chip8->registers.V[x] + chip8->registers.V[y];
        chip8->registers.V[0x0f] = false;
        if (tmp > 0xff)
        {
            chip8->registers.V[0x0f] = true;
        }
        chip8->registers.V[x] = tmp;
        break;
    case 0x05: // 8xy5 - SUB Vx, Vy //Set Vx = Vx - Vy, set VF = NOT borrow.
        tmp = chip8->registers.V[x] - chip8->registers.V[y];
        chip8->registers.V[0x0f] = false;
        if (tmp > 0)
        {
            chip8->registers.V[0x0f] = true;
        }
        chip8->registers.V[x] = tmp;
        break;
    case 0x06: // 8xy6 - SHR Vx {, Vy} //Set Vx = Vx SHR 1.
        chip8->registers.V[0x0f] = chip8->registers.V[x] & 0x01;
        chip8->registers.V[x] /= 2;
        break;
    case 0x07: // 8xy7 - SUBN Vx, Vy //Set Vx = Vy - Vx, set VF = NOT borrow..
        chip8->registers.V[0x0f] = chip8->registers.V[y] > chip8->registers.V[x];
        chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
        break;
    case 0x0E: // 8xyE - SHL Vx {, Vy}  //Set Vx = Vx SHL 1.
        chip8->registers.V[0x0f] = chip8->registers.V[x] & 0b10000000;
        chip8->registers.V[x] *= 2;
        break;
    }
}

static void chip8_exec_extended(struct chip8 *chip8, unsigned short opcode) // bitxise operation
{
    unsigned short nnn = opcode & 0x0fff; // care about the last 12 bits
    unsigned short x = (opcode >> 8) & 0x000f;
    unsigned short kk = opcode & 0x00ff;
    unsigned short y = (opcode >> 4) & 0x000f;
    unsigned short n = opcode & 0x000f;
    switch (opcode & 0xf000) // opcode & 0xf000, can get 1xxx, 2xxx, 3xxx,....,f
    {
    case 0x1000: // JP addr
        chip8->registers.PC = nnn;
        break;
    case 0x2000: // call subroutine   //The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
        chip8_stack_push(chip8, chip8->registers.PC);
        chip8->registers.PC = nnn;

        break;
    case 0x3000: // 3xkk - SE Vx, byte  //The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
        if (chip8->registers.V[x] == kk)
        {
            chip8->registers.PC += 2;
        }
        break;
    case 0x4000: // 4xkk - SNE Vx, byte, byte  //The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2..
        if (chip8->registers.V[x] != kk)
        {
            chip8->registers.PC += 2;
        }
        break;
    case 0x5000: // 5xy0 - SE Vx, Vy  //The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
        if (chip8->registers.V[x] == chip8->registers.V[y])
        {
            chip8->registers.PC += 2;
        }
        break;
    case 0x6000: // 6xkk - LD Vx, byte  //The interpreter puts the value kk into register Vx.
        chip8->registers.V[x] = kk;
        break;
    case 0x7000: // 7xkk - ADD Vx, byte  //Adds the value kk to the value of register Vx, then stores the result in Vx.
        chip8->registers.V[x] += kk;
        break;
    case 0x8000: // 8xy0 - LD Vx, Vy  //Stores the value of register Vy in register Vx.
        chip8_exec_extended_eight(chip8, opcode);
        break;
    case 0x9000: // 9xy0 - SNE Vx, Vy  //Skip next instruction if Vx != Vy.
        if (chip8->registers.V[x] != chip8->registers.V[y])
        {
            chip8->registers.PC += 2;
        }
        break;
    case 0xA000: // Annn - LD I, addr //Set I = nnn
        chip8->registers.I = nnn;
        break;
    case 0xB000: // Bnnn - JP V0, addr //Jump to location nnn + V0
        chip8->registers.PC = nnn + chip8->registers.V[0x00];
        break;
    case 0xC000: // Cxkk - RND Vx, byte //Set Vx = random byte AND kk
        srand(clock());
        chip8->registers.V[x] = (rand() % 255) + kk;
        break;
    case 0xD000: // Dxyn - DRW Vx, Vy, nibble //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision. // Dxyn, coord x = Vx, corrd y = Vy, height = n
    {
        const char *sprite = (const char *)&chip8->memory.memory[chip8->registers.I]; //*sprite knows the latction of location I => because starting at memory location I
        chip8->registers.V[0x0f] = chip8_screen_draw_sprite(&chip8->screen, chip8->registers.V[x], chip8->registers.V[y], sprite, n);
    }
    break;
    case 0xE000:
    {
        switch (opcode & 0x00ff)
        {
        case 0x9E: // Ex9E - SKP Vx      //Skip next instruction if key with the value of Vx is pressed.
            if (chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
            {
                chip8->registers.PC += 2;
            }
            break;
        case 0xA1: // ExA1 - SKNP Vx //Skip next instruction if key with the value of Vx is not pressed.
            if (!chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
            {
                chip8->registers.PC += 2;
            }
            break;
        }
    }
    break;
    case 0xF000:
        chip8_exec_extended_F(chip8, opcode);
        break;
    }
}

void chip8_exec(struct chip8 *chip8, unsigned short opcode)
{
    switch (opcode)
    {
    case 0x00E0: // clear screen -> chip8screen.h
        chip8_screen_clear(&chip8->screen);

        break;
    case 0X00EE:                                      // return form the subrutine -> chip8stack.h // The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
        chip8->registers.PC = chip8_stack_pop(chip8); // pop out addr

        break;
    default:
        chip8_exec_extended(chip8, opcode);
    }
}
