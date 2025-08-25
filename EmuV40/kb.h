#ifndef KB_H
#define KB_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

extern bool keys[256];

void handle_keyboard(bool down, SDL_Keycode sym, SDL_Scancode scancode);

#endif