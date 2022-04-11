#include "chip8screen.h"
#include <assert.h>
#include <memory.h>

static void chip8_screen_check_bounds(int x, int y)
{
    assert(x >= 0 && x < CHIP8_WIDTH && y >= 0 && y < CHIP8_HEIGHT);
}

void chip8_screen_set(struct chip8_screen *screen, int x, int y)
{
    chip8_screen_check_bounds(x, y);
    screen->pixels[y][x] = true;
}

bool chip8_screen_is_set(struct chip8_screen *screen, int x, int y)
{
    chip8_screen_check_bounds(x, y);
    return screen->pixels[y][x];
}

bool chip8_screen_draw_sprite(struct chip8_screen *screen, int x, int y, const char *sprite, int num)
{
    bool pixel_collison = false;
    for (int ly = 0; ly < num; ly++)
    {
        char c = sprite[ly];           // 8 bits = 1 byte, char == 1bytes
        for (int lx = 0; lx < 8; lx++) // 1 sprite == 8bits
        {
            if ((c & (0b10000000 >> lx)) == 0) // 10000000 >> 1 == 1000000, from 10000000 -> 1000000 -> 100000 -> ... -> 1, "1" is check bit of c exit or not
            {
                continue;
            }
            if (screen->pixels[(ly + y) % CHIP8_HEIGHT][(lx + x) % CHIP8_WIDTH] == 1)
            {
                pixel_collison = true;
            }
            screen->pixels[(ly + y) % CHIP8_HEIGHT][(lx + x) % CHIP8_WIDTH] ^= true; // XOR, 1^1 = 0, 1^0 = 1
        }
    }
    return pixel_collison;
}

void chip8_screen_clear(struct chip8_screen *screen) // pixels set to 0 == clear the screen
{
    memset(screen->pixels, 0, sizeof(screen->pixels));
}
