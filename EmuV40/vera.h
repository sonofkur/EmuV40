#ifndef VERA_H
#define VERA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include "config.h"

#define COMPOSER_SLOTS 4*64
#define NUM_LAYERS 2

#define APPROX_TITLEBAR_HEIGHT 30
#define PATH_MAX 255

#define VERA_VERSION_MAJOR  47
#define VERA_VERSION_MINOR  0
#define VERA_VERSION_PATCH  2

#define ADDR_VRAM_START     0x00000
#define ADDR_VRAM_END       0x10000
#define ADDR_PSG_START      0x1F9C0
#define ADDR_PSG_END        0x1FA00
#define ADDR_PALETTE_START  0x1FA00
#define ADDR_PALETTE_END    0x1FC00
#define ADDR_SPRDATA_START  0x1FC00
#define ADDR_SPRDATA_END    0x20000

#define NUM_SPRITES 128

// both VGA and NTSC
#define SCAN_HEIGHT 525
#define PIXEL_FREQ 25.0

// VGA
#define VGA_SCAN_WIDTH 800
#define VGA_Y_OFFSET 0

// NTSC: 262.5 lines per frame, lower field first
#define NTSC_HALF_SCAN_WIDTH 794
#define NTSC_X_OFFSET 270
#define NTSC_Y_OFFSET_LOW 42
#define NTSC_Y_OFFSET_HIGH 568
#define TITLE_SAFE_X 0.067
#define TITLE_SAFE_Y 0.05

// visible area we're drawing
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_RAM_OFFSET 0x00000

#define LSHORTCUT_KEY SDL_SCANCODE_LCTRL
#define RSHORTCUT_KEY SDL_SCANCODE_RCTRL

// When rendering a layer line, we can amortize some of the cost by calculating multiple pixels at a time.
#define LAYER_PIXELS_PER_ITERATION 8

#define MAX(a,b) ((a) > (b) ? a : b)

#define REG_CTRL			0x00
#define REG_IEN				0x01
#define REG_IRQ_LINE_L		0x02
#define REG_HSCALE			0x03
#define REG_VSCALE			0x04
#define REG_BORDER_COLOR	0x05
#define REG_HSTART			0x06
#define REG_HSTOP			0x07
#define REG_VSTART			0x08
#define REG_VSTOP			0x09
#define REG_L0_CONFIG		0x0A
#define REG_L0_MAP_BASE		0x0B
#define REG_L0_TILE_BASE	0x0C
#define REG_L0_HSCROLL_L	0x0D
#define REG_L0_HSCROLL_H	0x0E
#define REG_L0_VSCROLL_L	0x0F
#define REG_L0_VSCROLL_H	0x10
#define REG_L1_CONFIG		0x11
#define REG_L1_MAP_BASE		0x12
#define REG_L1_TILE_BASE	0x13
#define REG_L1_HSCROLL_L	0x14
#define REG_L1_HSCROLL_H	0x15
#define REG_L1_VSCROLL_L	0x16
#define REG_L1_VSCROLL_H	0x17
#define REG_ADDR_INC		0x18


typedef struct video_layer_properties {
	uint8_t color_depth;
	uint32_t map_base;
	uint32_t tile_base;

	bool text_mode;
	bool text_mode_256c;
	bool tile_mode;
	bool bitmap_mode;

	uint16_t hscroll;
	uint16_t vscroll;

	uint8_t  mapw_log2;
	uint8_t  maph_log2;
	uint16_t tilew;
	uint16_t tileh;
	uint8_t  tilew_log2;
	uint8_t  tileh_log2;

	uint16_t mapw_max;
	uint16_t maph_max;
	uint16_t tilew_max;
	uint16_t tileh_max;
	uint16_t layerw_max;
	uint16_t layerh_max;

	uint8_t tile_size_log2;

	int min_eff_x;
	int max_eff_x;

	uint8_t bits_per_pixel;
	uint8_t first_color_pos;
	uint8_t color_mask;
	uint8_t color_fields_max;
}VERA_LAYER;

typedef struct video_sprite_properties {
	int8_t sprite_zdepth;
	uint8_t sprite_collision_mask;

	int16_t sprite_x;
	int16_t sprite_y;
	uint8_t sprite_width_log2;
	uint8_t sprite_height_log2;
	uint8_t sprite_width;
	uint8_t sprite_height;

	bool hflip;
	bool vflip;

	uint8_t color_mode;
	uint32_t sprite_address;

	uint16_t palette_offset;
}VERA_SPRITE;

typedef struct video_palette {
	uint32_t entries[256];
	bool dirty;
}VERA_PALETTE;



typedef struct structvera {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* sdlTexture;

	uint8_t video_ram[0x10000];
	uint8_t palette[256 * 2];
	uint8_t sprite_data[128][8];

	uint32_t io_addr;
	uint8_t io_rddata;
	uint8_t io_inc;
	uint8_t io_dcsel;
	uint8_t ien;
	uint8_t isr;
	uint16_t irq_line;
	uint8_t reg_layer[2][7];

	/*uint8_t reg_composer[COMPOSER_SLOTS];
	uint8_t prev_reg_composer[2][COMPOSER_SLOTS];*/
	
	uint8_t vram_reg_sel;
	uint8_t curr_reg;
	uint8_t registers[256];
	uint8_t prev_registers[2][256];

	uint8_t layer_line[2][SCREEN_WIDTH];
	uint8_t sprite_line_col[SCREEN_WIDTH];
	uint8_t sprite_line_z[SCREEN_WIDTH];
	uint8_t sprite_line_mask[SCREEN_WIDTH];
	uint8_t sprite_line_collisions;
	bool layer_line_enable[2];
	bool old_layer_line_enable[2];
	bool old_sprite_line_enable;
	bool sprite_line_enable;

	VERA_LAYER layer_properties[NUM_LAYERS];
	VERA_LAYER prev_layer_properties[2][NUM_LAYERS];
	VERA_SPRITE sprite_properties[128];
	VERA_PALETTE video_palette;

	uint8_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
	float vga_scan_pos_x;
	uint16_t vga_scan_pos_y;
	float ntsc_half_cnt;
	uint16_t ntsc_scan_pos_y;
	int frame_count;
	char window_title[255];

	
}VERA;

bool vera_init(VERA* vera, int window_scale, float screen_x_scale, char* quality, bool fullscreen, float opacity);
void vera_reset(VERA* vera);
bool vera_step(VERA* vera, float mhz, uint32_t steps, bool midline);
bool vera_update(VERA* vera);
void vera_end(VERA* vera);
bool vera_get_irq_out(VERA* vera);
void video_save(VERA* vera, SDL_RWops* f);
uint8_t vera_read(VERA* vera, uint16_t reg);
void vera_write(VERA* vera, uint16_t reg, uint8_t value);
void video_update_title(VERA* vera, const char* window_title);

// For debugging purposes only:
uint8_t video_space_read(VERA* vera, uint32_t address);
void video_space_write(VERA* vera, uint32_t address, uint8_t value);

bool video_is_tilemap_address(VERA* vera, uint32_t addr);
bool video_is_tiledata_address(VERA* vera, uint32_t addr);
bool video_is_special_address(uint32_t addr);

uint32_t video_get_address(VERA* vera);

//void print_palette_fgpa();
//void export_palette_to_24bit();
void vera_load_default_font(uint32_t addr);
#endif
