#include "chip8memory.h"
#include <assert.h>

static void chip8_is_memory_in_bounds(int index)
{
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

void chip8_memory_set(struct chip8_memory *memory, int index, unsigned char val)
{
    chip8_is_memory_in_bounds(index);
    memory->memory[index] = val;
}
unsigned char chip8_memory_get(struct chip8_memory *memory, int index) // return one byte
{
    chip8_is_memory_in_bounds(index);
    return memory->memory[index];
}

// All instructions are 2 bytes long and are stored most-significant-byte first.

unsigned short chip8_memory_get_short(struct chip8_memory *memory, int index)
{
    // according to the chip8_memory_get function, get one byte by one time
    unsigned char byte1 = chip8_memory_get(memory, index);
    unsigned char byte2 = chip8_memory_get(memory, index + 1);
    // merge two byte
    return (byte1 << 8 | byte2);
}
