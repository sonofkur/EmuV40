#ifndef VERA_H
#define VERA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include "glue.h"

bool vera_init(int window_scale, float screen_x_scale, char* quality, bool fullscreen, float opacity);
void vera_reset(void);
bool vera_step(float mhz, uint32_t steps, bool midline);
bool vera_update(void);
void vera_end(void);
bool vera_get_irq_out(void);
void video_save(SDL_RWops* f);
uint8_t vera_read(uint16_t reg);
void vera_write(uint16_t reg, uint8_t value);
void video_update_title(const char* window_title);

// For debugging purposes only:
uint8_t video_space_read(uint32_t address);
void video_space_write(uint32_t address, uint8_t value);

bool video_is_tilemap_address(uint32_t addr);
bool video_is_tiledata_address(uint32_t addr);
bool video_is_special_address(uint32_t addr);

uint32_t video_get_address(uint8_t sel);
uint8_t video_get_dc_value(uint8_t reg);

//void print_palette_fgpa();
//void export_palette_to_24bit();
//void vera_load_default_font(uint32_t addr);
#endif
