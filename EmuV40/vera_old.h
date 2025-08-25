//#ifndef VERA_H
//#define VERA_H
//
//#include <stdbool.h>
//#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <SDL.h>
//#include "glue.h"
//
//#define ADDR_VRAM_START     0x0000
//#define ADDR_VRAM_END       0xFFFF
//#define ADDR_PALETTE_START  0xFE00
//#define ADDR_PALETTE_END    0xFFFF
//#define ADDR_SPRDATA_START  0xF900
//#define ADDR_SPRDATA_END    0xFCFF
//#define ADDR_REGS_START		0xFD00
//#define ADDR_REGS_END		0xFDFF
//
//#define NUM_SPRITES 128
//
//// both VGA and NTSC
//#define SCAN_HEIGHT 525
//#define PIXEL_FREQ 25.0
//
//// VGA
//#define VGA_SCAN_WIDTH 800
//#define VGA_Y_OFFSET 0
//
//// NTSC: 262.5 lines per frame, lower field first
//#define NTSC_HALF_SCAN_WIDTH 794
//#define NTSC_X_OFFSET 270
//#define NTSC_Y_OFFSET_LOW 42
//#define NTSC_Y_OFFSET_HIGH 568
//#define TITLE_SAFE_X 0.067
//#define TITLE_SAFE_Y 0.05
//
//// visible area we're drawing
//#define SCREEN_WIDTH 640
//#define SCREEN_HEIGHT 480
//
//#define SCREEN_RAM_OFFSET 0x0000
//
//#define LSHORTCUT_KEY SDL_SCANCODE_LCTRL
//#define RSHORTCUT_KEY SDL_SCANCODE_RCTRL
//
//#define APPROX_TITLEBAR_HEIGHT 30
//
//#define LAYER_PIXELS_PER_ITERATION 8
//
//#define MAX(a,b) ((a) > (b) ? a : b)
//
//#define NUM_OF_REGISTERS 256
//#define NUM_LAYERS 2
//
//enum ppu_registers {
//	PPU_REG_VIDEO = 0,
//	PPU_REG_HSCALE,
//	PPU_REG_VSCALE,
//	PPU_REG_BORDER_COLOR,
//	PPU_REG_HSTART,
//	PPU_REG_HSTOP,
//	PPU_REG_VSTART,
//	PPU_REG_VSTOP,
//	PPU_REG_L0_CONFIG,
//	PPU_REG_L0_MAP_BASE,
//	PPU_REG_L0_TILE_BASE,
//	PPU_REG_L0_HSCROLL_L,
//	PPU_REG_L0_HSCROLL_H,
//	PPU_REG_L0_VSCROLL_L,
//	PPU_REG_L0_VSCROLL_H,
//	PPU_REG_L1_CONFIG,
//	PPU_REG_L1_MAP_BASE,
//	PPU_REG_L1_TILE_BASE,
//	PPU_REG_L1_HSCROLL_L,
//	PPU_REG_L1_HSCROLL_H,
//	PPU_REG_L1_VSCROLL_L,
//	PPU_REG_L1_VSCROLL_H,
//	PPU_REG_IEN,
//	PPU_REG_IRQ_LINE,
//	PPU_REG_VRAM_ACCESS
//};
//
//struct video_layer_properties
//{
//	uint8_t color_depth;
//	uint32_t map_base;
//	uint32_t tile_base;
//
//	bool text_mode;
//	bool text_mode_256c;
//	bool tile_mode;
//	bool bitmap_mode;
//
//	uint16_t hscroll;
//	uint16_t vscroll;
//
//	uint8_t  mapw_log2;
//	uint8_t  maph_log2;
//	uint16_t tilew;
//	uint16_t tileh;
//	uint8_t  tilew_log2;
//	uint8_t  tileh_log2;
//
//	uint16_t mapw_max;
//	uint16_t maph_max;
//	uint16_t tilew_max;
//	uint16_t tileh_max;
//	uint16_t layerw_max;
//	uint16_t layerh_max;
//
//	uint8_t tile_size_log2;
//
//	int min_eff_x;
//	int max_eff_x;
//
//	uint8_t bits_per_pixel;
//	uint8_t first_color_pos;
//	uint8_t color_mask;
//	uint8_t color_fields_max;
//};
//
//struct video_sprite_properties
//{
//	int8_t sprite_zdepth;
//	uint8_t sprite_collision_mask;
//
//	int16_t sprite_x;
//	int16_t sprite_y;
//	uint8_t sprite_width_log2;
//	uint8_t sprite_height_log2;
//	uint8_t sprite_width;
//	uint8_t sprite_height;
//
//	bool hflip;
//	bool vflip;
//
//	uint8_t color_mode;
//	uint32_t sprite_address;
//
//	uint16_t palette_offset;
//};
//
//struct video_palette
//{
//	uint32_t entries[256];
//	bool dirty;
//};
//
//
//typedef struct vera {
//	uint8_t regs[NUM_OF_REGISTERS];
//	uint8_t prev_regs[2][NUM_OF_REGISTERS];
//	uint8_t layer_line[2][SCREEN_WIDTH];
//	uint8_t sprite_line_col[SCREEN_WIDTH];
//	uint8_t sprite_line_z[SCREEN_WIDTH];
//	uint8_t sprite_line_mask[SCREEN_WIDTH];
//	uint8_t sprite_line_collisions;
//	bool layer_line_enable[2];
//	bool old_layer_line_enable[2];
//	bool old_sprite_line_enable;
//	bool sprite_line_enable;
//
//	uint8_t vram[0x10000];
//	uint8_t palette[256 * 2];
//	uint8_t sprite_data[128][8];
//
//	uint32_t io_addr;
//	uint8_t io_rddata;
//	uint8_t io_inc;
//	uint8_t isr;
//	uint16_t irq_line;
//
//	float vga_scan_pos_x;
//	uint16_t vga_scan_pos_y;
//	float ntsc_half_cnt;
//	uint16_t ntsc_scan_pos_y;
//	int frame_count;
//
//	uint8_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
//
//	struct video_layer_properties layer_properties[NUM_LAYERS];
//	struct video_layer_properties prev_layer_properties[2][NUM_LAYERS];
//
//	struct video_sprite_properties sprite_properties[NUM_SPRITES];
//
//	struct video_palette video_palette;
//}VERA;
//
//
//bool vera_init(int window_scale, int screen_x_scale, char* quality, bool fullscreen, float opacity);
//void vera_reset(void);
//bool vera_step(float mhz, uint32_t steps, bool midline);
//bool vera_update(void);
//void vera_destroy(void);
//bool vera_get_irq_out(void);
//void vera_save(SDL_RWops* f);
//uint8_t vera_read(uint8_t reg, bool debugOn);
//void vera_write(uint8_t reg, uint8_t value);
//void vera_update_title(const char* window_title);
//bool vera_load_default_font(uint32_t addr);
//
//extern VERA vera;
//
//#endif 