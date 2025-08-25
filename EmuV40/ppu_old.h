//#ifndef PPU_H
//#define PPU_H
//
//#include <SDL.h>
//#include <stdbool.h>
//
//#define MAX_SPRITE_COUNT	64
//#define SPRITE_LIMIT		16
//#define PAT_START_ADDR		0x0000
//#define NAME_START_ADDR		0x8000
//#define OAM_START_ADDR		0xC000
//#define PAL_START_ADDR		0xF000
//
//#define NUM_OF_REGS 32
//
//#define LSHORTCUT_KEY SDL_SCANCODE_LCTRL
//#define RSHORTCUT_KEY SDL_SCANCODE_RCTRL
//
//typedef struct struct_sprite
//{
//	uint8_t x;
//	uint8_t y;
//	uint8_t attr;
//	uint8_t id;
//}SPRITE;
//
//typedef struct struct_loopy
//{
//	uint8_t coarse_x;
//	uint8_t coarse_y;
//	uint8_t nametable_x;
//	uint8_t nametable_y;
//	uint8_t fine_y;
//}LOOPY;
//
//typedef struct struct_bg
//{
//	uint32_t shifter_pattern_lo;
//	uint32_t shifter_pattern_hi;
//	uint32_t shifter_attr_lo;
//	uint32_t shifter_attr_hi;
//	uint32_t next_tile_lsb;
//	uint32_t next_tile_msb;
//	uint32_t next_tile_attr;
//	uint32_t next_tile_id;
//	uint8_t fine_x;
//	uint32_t pixel;
//	uint32_t palette;
//	LOOPY loopy_v;
//	LOOPY loopy_t;
//}BACKGROUND;
//
//struct structppu {
//	uint8_t registers[NUM_OF_REGS];
//	BACKGROUND backgrounds[2];
//	uint64_t cycles;
//	int cycle;
//	int scanline;
//	uint8_t addr_reg_flag;
//	//uint8_t addr_latch;
//	uint16_t vram_addr;
//	uint8_t current_reg;
//	int sprite_count;
//	bool sprite_zero_being_rendered;
//	bool sprite_zero_hit_possible;
//	SPRITE sprites[MAX_SPRITE_COUNT];
//	uint8_t ss_pattern_lo[MAX_SPRITE_COUNT];
//	uint8_t ss_pattern_hi[MAX_SPRITE_COUNT];
//	bool frame_complete;
//	uint8_t oam[MAX_SPRITE_COUNT*4];
//	uint8_t vram[0x10000];
//	uint32_t nmi_count;
//};
//
//extern struct structppu ppu;
//
//void ppu_init(int window_scale, char* quality, bool fullscreen);
//void ppu_reset(void);
//void ppu_soft_reset(void);
//bool ppu_step(uint32_t steps);
//bool ppu_render(void);
//void ppu_shutdown(void);
//void ppu_frame_start(void);
//bool ppu_load_default_font(uint16_t addr);
//bool ppu_load_bin(char* filename, uint16_t addr);
//
//void ppu_vram_write(uint16_t address, uint8_t value);
//uint8_t ppu_vram_read(uint16_t address);
//
//void ppu_load_test_sprites(void);
//void ppu_load_test_data(void);
//void ppu_write_register(uint8_t reg, uint8_t value);
//
//#endif