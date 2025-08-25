//#include "ppu.h"
//#include "cpu.h"
//#include "ports.h"
//#include <string.h>
////#include "i8259.h"
//#include <stdio.h>
//#include "glue.h"
//
//#define SCREEN_WIDTH			256
//#define SCREEN_HEIGHT			240
//
//#define PPU_VRAM_ADDR_LO		0
//#define PPU_VRAM_ADDR_HI		1
//#define PPU_DATA				2
//#define PPU_REGISTER_NUM		3
//
//#define PPU_MASK_BG0_LEFT_COL	0
//#define PPU_MASK_BG1_LEFT_COL	1
//#define PPU_MASK_SPR_LEFT_COL	2
//#define PPU_MASK_BG0_ENABLE		3
//#define PPU_MASK_BG1_ENABLE		4
//#define PPU_MASK_SPR_ENABLE		5
//
//#define PPU_REG_CONTROL			0
//#define PPU_REG_PATTERN			1
//#define PPU_REG_NAMETABLE		2
//#define PPU_REG_MASK			3
//#define PPU_REG_STATUS			4
//#define PPU_REG_BG0_SCROLL_X	5
//#define PPU_REG_BG0_SCROLL_Y	6
//#define PPU_REG_BG1_SCROLL_X	7
//#define PPU_REG_BG1_SCROLL_Y	8
//
//
//#define SET_BIT(n,b)    (n |= (1<<b))
//#define CLEAR_BIT(n,b)  (n &= ~(1<<b))
//#define TOGGLE_BIT(n,b) (n ^= (1<<b))
//#define CHECK_BIT(n,b)  ((n>>b) & 1)
//
//uint32_t palette_screen[] = {
//	0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
//	0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
//	0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
//	0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
//	0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
//	0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
//	0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
//	0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
//};
//
//// 1-bit font, needs to be converted to 2-bit for the ppu
//uint8_t default_ppu_font[] = {
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		//   0  nul
//			0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,		//   1  so
//			0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,		//   2  stx
//			0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,		//   3  etx
//			0x10, 0x38, 0x7C, 0xFE,	0x7C, 0x38, 0x10, 0x00,		//   4  eot
//			0x38, 0x7C, 0x38, 0xFE,	0xFE, 0x7C, 0x38, 0x7C,		//   5  enq
//			0x10, 0x10, 0x38, 0x7C,	0xFE, 0x7C, 0x38, 0x7C,		//   6  ack
//			0x00, 0x00, 0x18, 0x3C,	0x3C, 0x18, 0x00, 0x00,		//   7  bel
//			0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,		//   8  bs
//			0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,		//   9  t
//			0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,		//  10  lf
//			0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,		//  11  vt
//			0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,		//  12  ff
//			0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,		//  13  cr
//			0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,		//  14  so
//			0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,		//  15  si
//			0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,		//  16  dle
//			0x02, 0x0E, 0x3E, 0xFE,	0x3E, 0x0E, 0x02, 0x00,		//  17  dc1
//			0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18,		//  18  dc2
//			0x66, 0x66, 0x66, 0x66,	0x66, 0x00, 0x66, 0x00,		//  19  dc3
//			0x7F, 0xDB, 0xDB, 0x7B,	0x1B, 0x1B, 0x1B, 0x00,		//  20x  dc4
//			0x3E, 0x63, 0x38, 0x6C,	0x6C, 0x38, 0xCC, 0x78,		//  21  nak
//			0x00, 0x00, 0x00, 0x00,	0x7E, 0x7E, 0x7E, 0x00,		//  22  syn
//			0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,		//  23  etb
//			0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,		//  24  can
//			0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,		//  25  em
//			0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,		//  26  sub
//			0x00, 0x30, 0x60, 0xFE,	0x60, 0x30, 0x00, 0x00,		//  27  esc
//			0x00, 0x00, 0xC0, 0xC0,	0xC0, 0xFE, 0x00, 0x00,		//  28  fs
//			0x00, 0x24, 0x66, 0xFF,	0x66, 0x24, 0x00, 0x00,		//  29  gs
//			0x00, 0x18, 0x3C, 0x7E,	0xFF, 0xFF, 0x00, 0x00,		//  30  rs
//			0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00,		//  31  us
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		//  32  space
//			0x30, 0x78, 0x78, 0x30,	0x30, 0x00, 0x30, 0x00,		//  33  !
//			0x6C, 0x6C, 0x6C, 0x00,	0x00, 0x00, 0x00, 0x00,		//  34  "
//			0x6C, 0x6C, 0xFE, 0x6C,	0xFE, 0x6C, 0x6C, 0x00,		//  35  #
//			0x30, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00,		//  36  $
//			0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00,		//  37  %
//			0x38, 0x6C, 0x38, 0x76,	0xDC, 0xCC, 0x76, 0x00,		//  38  &
//			0x60, 0x60, 0xC0, 0x00,	0x00, 0x00, 0x00, 0x00,		//  39  '
//			0x18, 0x30, 0x60, 0x60,	0x60, 0x30, 0x18, 0x00,		//  40  (
//			0x60, 0x30, 0x18, 0x18,	0x18, 0x30, 0x60, 0x00,		//  41  )
//			0x00, 0x66, 0x3C, 0xFF,	0x3C, 0x66, 0x00, 0x00,		//  42  *
//			0x00, 0x30, 0x30, 0xFC,	0x30, 0x30, 0x00, 0x00,		//  43  +
//			0x00, 0x00, 0x00, 0x00,	0x00, 0x30, 0x30, 0x60,		//  44  ,
//			0x00, 0x00, 0x00, 0xFC,	0x00, 0x00, 0x00, 0x00,		//  45  -
//			0x00, 0x00, 0x00, 0x00,	0x00, 0x30, 0x30, 0x00,		//  46  .
//			0x06, 0x0C, 0x18, 0x30,	0x60, 0xC0, 0x80, 0x00,		//  47  /
//			0x7C, 0xC6, 0xCE, 0xDE,	0xF6, 0xE6, 0x7C, 0x00,		//  48  0
//			0x30, 0x70, 0x30, 0x30,	0x30, 0x30, 0xFC, 0x00,		//  49  1
//			0x78, 0xCC, 0x0C, 0x38,	0x60, 0xCC, 0xFC, 0x00,		//  50  2
//			0x78, 0xCC, 0x0C, 0x38,	0x0C, 0xCC, 0x78, 0x00,		//  51  3
//			0x1C, 0x3C, 0x6C, 0xCC,	0xFE, 0x0C, 0x1E, 0x00,		//  52  4
//			0xFC, 0xC0, 0xF8, 0x0C,	0x0C, 0xCC, 0x78, 0x00,		//  53  5
//			0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00,		//  54  6
//			0xFC, 0xCC, 0x0C, 0x18,	0x30, 0x30, 0x30, 0x00,		//  55  7
//			0x78, 0xCC, 0xCC, 0x78,	0xCC, 0xCC, 0x78, 0x00,		//  56  8
//			0x78, 0xCC, 0xCC, 0x7C,	0x0C, 0x18, 0x70, 0x00,		//  57  9
//			0x00, 0x30, 0x30, 0x00,	0x00, 0x30, 0x30, 0x00,		//  58  :
//			0x00, 0x30, 0x30, 0x00,	0x00, 0x30, 0x30, 0x60,		//  59  //
//			0x18, 0x30, 0x60, 0xC0,	0x60, 0x30, 0x18, 0x00,		//  60x  <
//			0x00, 0x00, 0xFC, 0x00,	0x00, 0xFC, 0x00, 0x00,		//  61  =
//			0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,		//  62  >
//			0x78, 0xCC, 0x0C, 0x18,	0x30, 0x00, 0x30, 0x00,		//  63  ?
//			0x7C, 0xC6, 0xDE, 0xDE,	0xDE, 0xC0, 0x78, 0x00,		//  64  @
//			0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00,		//  65  A
//			0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,		//  66  B
//			0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00,		//  67  C
//			0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00,		//  68  D
//			0xFE, 0x62, 0x68, 0x78,	0x68, 0x62, 0xFE, 0x00,		//  69  E
//			0xFE, 0x62, 0x68, 0x78,	0x68, 0x60, 0xF0, 0x00,		//  70x  F
//			0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00,		//  71  G
//			0xCC, 0xCC, 0xCC, 0xFC,	0xCC, 0xCC, 0xCC, 0x00,		//  72  
//			0x78, 0x30, 0x30, 0x30,	0x30, 0x30, 0x78, 0x00,		//  73  I
//			0x1E, 0x0C, 0x0C, 0x0C,	0xCC, 0xCC, 0x78, 0x00,		//  74  J
//			0xE6, 0x66, 0x6C, 0x78,	0x6C, 0x66, 0xE6, 0x00,		//  75  K
//			0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00,		//  76  L
//			0xC6, 0xEE, 0xFE, 0xFE,	0xD6, 0xC6, 0xC6, 0x00,		//  77  M
//			0xC6, 0xE6, 0xF6, 0xDE,	0xCE, 0xC6, 0xC6, 0x00,		//  78  N
//			0x38, 0x6C, 0xC6, 0xC6,	0xC6, 0x6C, 0x38, 0x00,		//  79  O
//			0xFC, 0x66, 0x66, 0x7C,	0x60, 0x60, 0xF0, 0x00,		//  80x  P
//			0x78, 0xCC, 0xCC, 0xCC,	0xDC, 0x78, 0x1C, 0x00,		//  81  Q
//			0xFC, 0x66, 0x66, 0x7C,	0x6C, 0x66, 0xE6, 0x00,		//  82  R
//			0x78, 0xCC, 0xE0, 0x70,	0x1C, 0xCC, 0x78, 0x00,		//  83  S
//			0xFC, 0xB4, 0x30, 0x30,	0x30, 0x30, 0x78, 0x00,		//  84  T
//			0xCC, 0xCC, 0xCC, 0xCC,	0xCC, 0xCC, 0xFC, 0x00,		//  85  U
//			0xCC, 0xCC, 0xCC, 0xCC,	0xCC, 0x78, 0x30, 0x00,		//  86  V
//			0xC6, 0xC6, 0xC6, 0xD6,	0xFE, 0xEE, 0xC6, 0x00,		//  87  W
//			0xC6, 0xC6, 0x6C, 0x38,	0x38, 0x6C, 0xC6, 0x00,		//  88  X
//			0xCC, 0xCC, 0xCC, 0x78,	0x30, 0x30, 0x78, 0x00,		//  89  Y
//			0xFE, 0xC6, 0x8C, 0x18,	0x32, 0x66, 0xFE, 0x00,		//  90x  Z
//			0x78, 0x60, 0x60, 0x60,	0x60, 0x60, 0x78, 0x00,		//  91  [
//			0xC0, 0x60, 0x30, 0x18,	0x0C, 0x06, 0x02, 0x00,		//  92  backslas
//			0x78, 0x18, 0x18, 0x18,	0x18, 0x18, 0x78, 0x00,		//  93  ]
//			0x10, 0x38, 0x6C, 0xC6,	0x00, 0x00, 0x00, 0x00,		//  94  ^
//			0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0xFF,		//  95  _
//			0x30, 0x30, 0x18, 0x00,	0x00, 0x00, 0x00, 0x00,		//  96  `
//			0x00, 0x00, 0x78, 0x0C,	0x7C, 0xCC, 0x76, 0x00,		//  97  a
//			0xE0, 0x60, 0x60, 0x7C,	0x66, 0x66, 0xDC, 0x00,		//  98  b
//			0x00, 0x00, 0x78, 0xCC,	0xC0, 0xCC, 0x78, 0x00,		//  99  c
//			0x1C, 0x0C, 0x0C, 0x7C,	0xCC, 0xCC, 0x76, 0x00,		// 10x0x  d
//			0x00, 0x00, 0x78, 0xCC,	0xFC, 0xC0, 0x78, 0x00,		// 10x1  e
//			0x38, 0x6C, 0x60, 0xF0,	0x60, 0x60, 0xF0, 0x00,		// 10x2  f
//			0x00, 0x00, 0x76, 0xCC,	0xCC, 0x7C, 0x0C, 0xF8,		// 10x3  g
//			0xE0, 0x60, 0x6C, 0x76,	0x66, 0x66, 0xE6, 0x00,		// 10x4  
//			0x30, 0x00, 0x70, 0x30,	0x30, 0x30, 0x78, 0x00,		// 10x5  i
//			0x0C, 0x00, 0x0C, 0x0C,	0x0C, 0xCC, 0xCC, 0x78,		// 10x6  j
//			0xE0, 0x60, 0x66, 0x6C,	0x78, 0x6C, 0xE6, 0x00,		// 10x7  k
//			0x70, 0x30, 0x30, 0x30,	0x30, 0x30, 0x78, 0x00,		// 10x8  l
//			0x00, 0x00, 0xCC, 0xFE,	0xFE, 0xD6, 0xC6, 0x00,		// 10x9  m
//			0x00, 0x00, 0xF8, 0xCC,	0xCC, 0xCC, 0xCC, 0x00,		// 110x  n
//			0x00, 0x00, 0x78, 0xCC,	0xCC, 0xCC, 0x78, 0x00,		// 111  o
//			0x00, 0x00, 0xDC, 0x66,	0x66, 0x7C, 0x60, 0xF0,		// 112  p
//			0x00, 0x00, 0x76, 0xCC,	0xCC, 0x7C, 0x0C, 0x1E,		// 113  q
//			0x00, 0x00, 0xDC, 0x76,	0x66, 0x60, 0xF0, 0x00,		// 114  r
//			0x00, 0x00, 0x7C, 0xC0,	0x78, 0x0C, 0xF8, 0x00,		// 115  s
//			0x10, 0x30, 0x7C, 0x30,	0x30, 0x34, 0x18, 0x00,		// 116  t
//			0x00, 0x00, 0xCC, 0xCC,	0xCC, 0xCC, 0x76, 0x00,		// 117  u
//			0x00, 0x00, 0xCC, 0xCC,	0xCC, 0x78, 0x30, 0x00,		// 118  v
//			0x00, 0x00, 0xC6, 0xD6, 0xFE, 0xFE, 0x6C, 0x00,		// 119  w
//			0x00, 0x00, 0xC6, 0x6C,	0x38, 0x6C, 0xC6, 0x00,		// 120x  x
//			0x00, 0x00, 0xCC, 0xCC,	0xCC, 0x7C, 0x0C, 0xF8,		// 121  y
//			0x00, 0x00, 0xFC, 0x98,	0x30, 0x64, 0xFC, 0x00,		// 122  z
//			0x1C, 0x30, 0x30, 0xE0,	0x30, 0x30, 0x1C, 0x00,		// 123  {
//			0x18, 0x18, 0x18, 0x00,	0x18, 0x18, 0x18, 0x00,		// 124  |
//			0xE0, 0x30, 0x30, 0x1C,	0x30, 0x30, 0xE0, 0x00,		// 125  }
//			0x76, 0xDC, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,		// 126  ~
//			0x00, 0x10, 0x38, 0x6C,	0xC6, 0xC6, 0xFE, 0x00		// 127  del
//};
//
//struct structppu ppu;
//
//static SDL_Window* window;
//static SDL_Renderer* renderer;
//static SDL_Texture* sdlTexture;
//
//static uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
//
//static bool is_fullscreen = false;
////bool mouse_grabbed = false;
//
//static uint8_t ppu_read(uint16_t portnum);
//static void ppu_write(uint16_t portnum, uint8_t value);
//void update_shifters(void);
//void load_background_shifters(void);
//void inc_scroll_x(void);
//void inc_scroll_y(void);
//void transfer_addr_x(void);
//void transfer_addr_y(void);
//void ppu_render_pixel(void);
//uint8_t flip_byte(uint8_t b);
//void ppu_vram_write(uint16_t address, uint8_t value);
//uint8_t ppu_vram_read(uint16_t address);
//
//uint16_t get_loopy(LOOPY* l);
//void set_loopy(LOOPY* l, uint16_t value);
//
//void ppu_init(int window_scale, char* quality, bool fullscreen) {
//
//	set_port_write_redirector(0xC0, 0xC3, &ppu_write);
//	set_port_read_redirector(0xC0, 0xC3, &ppu_read);
//
//	uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
//	uint32_t renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
//
//	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality);
//	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1"); // Grabs keyboard shortcuts from the system during window grab
//
//	window = SDL_CreateWindow("V40 Emulator", 100, 200, (int)(SCREEN_WIDTH * window_scale), (int)(SCREEN_HEIGHT * window_scale), window_flags);
//	renderer = SDL_CreateRenderer(window, -1, renderer_flags);
//	SDL_GL_SetSwapInterval(1);
//
//
//	//SDL_CreateWindowAndRenderer((int)(SCREEN_WIDTH * window_scale), (int)(SCREEN_HEIGHT * window_scale), window_flags, &window, &renderer);
//	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
//	//SDL_SetWindowPosition(window, 100, 200);
//	sdlTexture = SDL_CreateTexture(renderer,
//		SDL_PIXELFORMAT_RGB888,
//		SDL_TEXTUREACCESS_STREAMING,
//		SCREEN_WIDTH, SCREEN_HEIGHT);
//	if (fullscreen) {
//		is_fullscreen = true;
//		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
//	}
//}
//
//void ppu_shutdown(void) {
//	is_fullscreen = false;
//	SDL_SetWindowFullscreen(window, 0);
//	SDL_DestroyRenderer(renderer);
//	SDL_DestroyWindow(window);
//}
//
//void ppu_reset(void) {
//	memset((void*)&ppu, 0, sizeof(ppu));
//	ppu.scanline = -1;
//	ppu.frame_complete = false;
//}
//
//void ppu_soft_reset(void) {
//	uint8_t* temp;
//	temp = (uint8_t*)malloc(sizeof(uint8_t) * 0x10000);
//
//	if (temp == NULL)
//		return;
//
//	memcpy(temp, ppu.vram, 0x10000);
//
//	memset((void*)&ppu, 0, sizeof(ppu));
//	ppu.scanline = -1;
//	ppu.frame_complete = false;
//
//	memcpy(ppu.vram, temp, 0x10000);
//	free(temp);
//}
//
//void ppu_frame_start(void) {
//	ppu.scanline = -1;
//	ppu.cycle = 0;
//	ppu.frame_complete = false;
//}
//
//bool ppu_step(uint32_t steps) {
//
//	for (uint32_t i = 0; i < steps; i++) {
//
//		if (ppu.frame_complete)
//			return false;
//
//		// Background Rendering ========================================================
//		if (ppu.scanline >= -1 && ppu.scanline < 240) {
//
//			// "Odd Frame" cycle skip
//			if (ppu.scanline == 0 && ppu.cycle == 0) {
//				ppu.cycle = 1;
//			}
//
//			if (ppu.scanline == -1 && ppu.cycle == 1) {
//				ppu.frame_complete = false;
//				ppu.registers[PPU_REG_STATUS] = 0;
//
//				// Clear Shifters
//				for (int s = 0; s < ppu.sprite_count; s++) {
//					ppu.ss_pattern_lo[s] = 0;
//					ppu.ss_pattern_hi[s] = 0;
//				}
//			}
//
//			//main background rending loop
//			if ((ppu.cycle >= 2 && ppu.cycle < 258) || (ppu.cycle >= 321 && ppu.cycle < 338)) {
//
//				update_shifters();
//
//				// In these cycles we are collecting and working with visible data
//				// The "shifters" have been preloaded by the end of the previous
//				// scanline with the data for the start of this scanline. Once we
//				// leave the visible region, we go dormant until the shifters are
//				// preloaded for the next scanline.
//
//				// Fortunately, for background rendering, we go through a fairly
//				// repeatable sequence of events, every 2 clock cycles.
//				uint16_t data = 0;
//				switch ((ppu.cycle - 1) % 8) {
//
//					// nametable tile
//				case 0:
//					load_background_shifters();
//
//					data = (0x8000 | (get_loopy(&ppu.backgrounds[0].loopy_v) & 0x0FFF));
//					ppu.backgrounds[0].next_tile_id = ppu.vram[data];
//					data = (0xA000 | (get_loopy(&ppu.backgrounds[1].loopy_v) & 0x0FFF));
//					ppu.backgrounds[1].next_tile_id = ppu.vram[data];
//					break;
//
//					// attribute 
//				case 2:
//					data = (0x9000 | (get_loopy(&ppu.backgrounds[0].loopy_v) & 0x0FFF));
//					ppu.backgrounds[0].next_tile_attr = ppu.vram[data];
//					data = (0xB000 | (get_loopy(&ppu.backgrounds[1].loopy_v) & 0x0FFF));
//					ppu.backgrounds[1].next_tile_attr = ppu.vram[data];
//					break;
//
//					//pattern LSB
//				case 4:
//					data = ((ppu.registers[PPU_REG_PATTERN] & 0b111) << 12) + (ppu.backgrounds[0].next_tile_id << 4) + (ppu.backgrounds[0].loopy_v.fine_y);
//					ppu.backgrounds[0].next_tile_lsb = ppu.vram[data];
//					data = (((ppu.registers[PPU_REG_PATTERN] >> 3) & 0b111) << 12) + (ppu.backgrounds[1].next_tile_id << 4) + (ppu.backgrounds[1].loopy_v.fine_y);
//					ppu.backgrounds[1].next_tile_lsb = ppu.vram[data];
//					break;
//
//					//pattern MSB
//				case 6:
//					data = ((ppu.registers[PPU_REG_PATTERN] & 0b111) << 12) + (ppu.backgrounds[0].next_tile_id << 4) + (ppu.backgrounds[0].loopy_v.fine_y) + 8;
//					ppu.backgrounds[0].next_tile_msb = ppu.vram[data];
//					data = (((ppu.registers[PPU_REG_PATTERN] >> 3) & 0b111) << 12) + (ppu.backgrounds[1].next_tile_id << 4) + (ppu.backgrounds[1].loopy_v.fine_y) + 8;
//					ppu.backgrounds[1].next_tile_msb = ppu.vram[data];
//					break;
//
//					//inc x scroll
//				case 7:
//					inc_scroll_x();
//					break;
//				}
//			}
//
//			// End of a visible scanline, so increment downwards...
//			if (ppu.cycle == 256) {
//				inc_scroll_y();
//
//				// if h blank, IRQ
//				if (CHECK_BIT(ppu.registers[PPU_REG_CONTROL], 6)) {
//					//doirq(1);
//				}
//			}
//
//			//...and reset the x position
//			if (ppu.cycle == 257) {
//				load_background_shifters();
//				transfer_addr_x();
//			}
//
//			// Superfluous reads of tile id at end of scanline
//			if (ppu.cycle == 338 || ppu.cycle == 340) {
//				ppu.backgrounds[0].next_tile_id = ppu.vram[0x8000 | (get_loopy(&ppu.backgrounds[0].loopy_v) & 0x0FFF)];
//			}
//
//			// End of vertical blank period so reset the Y address ready for rendering
//			if (ppu.scanline == -1 && ppu.cycle >= 280 && ppu.cycle < 305) {
//				transfer_addr_y();
//			}
//		}
//		//===========================================================================
//
//		// Foreground Rendering ========================================================
//		// I'm gonna cheat a bit here, which may reduce compatibility, but greatly
//		// simplifies delivering an intuitive understanding of what exactly is going
//		// on. The PPU loads sprite information successively during the region that
//		// background tiles are not being drawn. Instead, I'm going to perform
//		// all sprite evaluation in one hit. THE NES DOES NOT DO IT LIKE THIS! This makes
//		// it easier to see the process of sprite evaluation.
//		if (ppu.cycle == 257 && ppu.scanline >= 0) {
//
//			for (int i = 0; i < MAX_SPRITE_COUNT; i++) {
//				ppu.sprites[i].y = 0xFF;
//				ppu.sprites[i].id = 0xFF;
//				ppu.sprites[i].attr = 0xFF;
//				ppu.sprites[i].x = 0xFF;
//			}
//
//			ppu.sprite_count = 0;
//
//			for (int i = 0; i < SPRITE_LIMIT; i++) {
//				ppu.ss_pattern_lo[i] = 0;
//				ppu.ss_pattern_hi[i] = 0;
//			}
//
//			uint32_t oam_entry = 0;
//
//			ppu.sprite_zero_hit_possible = false;
//
//			while (oam_entry < MAX_SPRITE_COUNT && ppu.sprite_count < (SPRITE_LIMIT + 1)) {
//				int diff = (ppu.scanline - (int)ppu.vram[OAM_START_ADDR + (oam_entry * 4)]);
//
//				if (diff >= 0 && diff < 8) {
//					if (ppu.sprite_count < SPRITE_LIMIT) {
//						if (oam_entry == 0) {
//							ppu.sprite_zero_hit_possible = true;
//						}
//
//						ppu.sprites[ppu.sprite_count].y = ppu.vram[OAM_START_ADDR + (oam_entry * 4)];
//						ppu.sprites[ppu.sprite_count].id = ppu.vram[OAM_START_ADDR + (oam_entry * 4 + 1)];
//						ppu.sprites[ppu.sprite_count].attr = ppu.vram[OAM_START_ADDR + (oam_entry * 4 + 2)];
//						ppu.sprites[ppu.sprite_count].x = ppu.vram[OAM_START_ADDR + (oam_entry * 4 + 3)];
//						ppu.sprite_count++;
//					}
//				}
//
//				oam_entry++;
//			}
//
//			if (ppu.sprite_count > SPRITE_LIMIT)
//				SET_BIT(ppu.registers[PPU_REG_STATUS], 5);
//			else
//				CLEAR_BIT(ppu.registers[PPU_REG_STATUS], 5);
//		}
//
//
//		// END OF SCANLINE
//		if (ppu.cycle == 340) {
//			for (int i = 0; i < ppu.sprite_count; i++) {
//				uint8_t sprite_pattern_bits_lo, sprite_pattern_bits_hi;
//				uint16_t sprite_pattern_addr_lo, sprite_pattern_addr_hi;
//				uint32_t sprPatTableAddr = ((ppu.registers[PPU_REG_PATTERN] >> 6) & 0x3) | ((ppu.registers[PPU_REG_MASK] & 0x80) >> 5);
//
//				// Determine the memory addresses that contain the byte of pattern data. We
//				// only need the lo pattern address, because the hi pattern address is always
//				// offset by 8 from the lo address.
//				// 8x8 Sprite Mode
//				if ((ppu.sprites[i].attr & 0x80) == 0) {
//					// Sprite is NOT flipped vertically, i.e. normal
//					sprite_pattern_addr_lo = (
//						(sprPatTableAddr << 12)  // Which Pattern Table? 0KB or 4KB offset
//						| (ppu.sprites[i].id << 4)  // Which Cell? Tile ID * 16 (16 bytes per tile)
//						| (ppu.scanline - ppu.sprites[i].y));
//				}
//				else {
//					sprite_pattern_addr_lo = (
//						(sprPatTableAddr << 12)  // Which Pattern Table? 0KB or 4KB offset
//						| (ppu.sprites[i].id << 4)  // Which Cell? Tile ID * 16 (16 bytes per tile)
//						| (7 - (ppu.scanline - ppu.sprites[i].y)));
//				}
//
//				sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;
//
//				sprite_pattern_bits_lo = ppu.vram[sprite_pattern_addr_lo];
//				sprite_pattern_bits_hi = ppu.vram[sprite_pattern_addr_hi];
//
//				// If the sprite is flipped horizontally, we need to flip the 
//				// pattern bytes. 
//				if ((ppu.sprites[i].attr & 0x40) == 0x40) {
//					sprite_pattern_bits_lo = flip_byte(sprite_pattern_bits_lo);
//					sprite_pattern_bits_hi = flip_byte(sprite_pattern_bits_hi);
//				}
//
//				ppu.ss_pattern_lo[i] = sprite_pattern_bits_lo;
//				ppu.ss_pattern_hi[i] = sprite_pattern_bits_hi;
//			}
//		}
//		//===========================================================================
//
//		if ((ppu.scanline <= 0 && ppu.scanline < 240 || ppu.scanline == -1) && ppu.cycle == 260 && (CHECK_BIT(ppu.registers[PPU_REG_MASK], 3) || CHECK_BIT(ppu.registers[PPU_REG_MASK], 4) || CHECK_BIT(ppu.registers[PPU_REG_MASK], 5))) {
//			//TODO scanline handler
//		}
//
//		// Post Render Scanline - Do Nothing!
//		if (ppu.scanline == 240) {}
//
//		if (ppu.scanline == 240 && ppu.cycle == 0) {
//			CLEAR_BIT(ppu.registers[PPU_REG_STATUS], 7);
//
//			if (CHECK_BIT(ppu.registers[PPU_REG_CONTROL], 7)) {
//				cpu.nmi = 1;
//				ppu.nmi_count++;
//			}
//		}
//		ppu_render_pixel();
//
//		ppu.cycle++;
//
//		if (ppu.cycle >= 341) {
//			ppu.cycle = 0;
//			ppu.scanline++;
//			if (ppu.scanline >= 261) {
//				ppu.scanline = -1;
//				ppu.frame_complete = true;
//			}
//		}
//
//		ppu.cycles++;
//	}
//
//	return ppu.frame_complete;
//}
//
//void ppu_render_pixel(void) {
//
//	// Background 0 =============================================================
//	ppu.backgrounds[0].pixel = 0;
//	ppu.backgrounds[0].palette = 0;
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE)) {
//		uint32_t bit_mux = 0x8000 >> ppu.backgrounds[0].fine_x;
//
//		uint32_t p0_pixel = (ppu.backgrounds[0].shifter_pattern_lo & bit_mux) > 0 ? 1 : 0;
//		uint32_t p1_pixel = (ppu.backgrounds[0].shifter_pattern_hi & bit_mux) > 0 ? 1 : 0;
//
//		ppu.backgrounds[0].pixel = (p1_pixel << 1) | p0_pixel;
//
//
//		uint32_t bg_pal0 = (ppu.backgrounds[0].shifter_attr_lo & bit_mux) > 0 ? 1 : 0;
//		uint32_t bg_pal1 = (ppu.backgrounds[0].shifter_attr_hi & bit_mux) > 0 ? 1 : 0;
//
//		ppu.backgrounds[0].palette = (bg_pal1 << 1) | bg_pal0;
//	}
//
//	// Background 1 =============================================================
//	ppu.backgrounds[1].pixel = 0;
//	ppu.backgrounds[1].palette = 0;
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE)) {
//		uint32_t bit_mux = 0x8000 >> ppu.backgrounds[1].fine_x;
//
//		uint32_t p0_pixel = (ppu.backgrounds[1].shifter_pattern_lo & bit_mux) > 0 ? 1 : 0;
//		uint32_t p1_pixel = (ppu.backgrounds[1].shifter_pattern_hi & bit_mux) > 0 ? 1 : 0;
//
//		ppu.backgrounds[1].pixel = (p1_pixel << 1) | p0_pixel;
//
//
//		uint32_t bg_pal0 = (ppu.backgrounds[1].shifter_attr_lo & bit_mux) > 0 ? 1 : 0;
//		uint32_t bg_pal1 = (ppu.backgrounds[1].shifter_attr_hi & bit_mux) > 0 ? 1 : 0;
//
//		ppu.backgrounds[1].palette = (bg_pal1 << 1) | bg_pal0;
//	}
//
//	// Foreground ================================================================
//	uint32_t fg_pixel = 0x00;   // The 2-bit pixel to be rendered
//	uint32_t fg_palette = 0x00; // The 3-bit index of the palette the pixel indexes
//	uint32_t fg_priority = 0x00;// A bit of the sprite attribute indicates if its more important than the background
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_ENABLE)) {
//		ppu.sprite_zero_being_rendered = false;
//
//		for (int i = 0; i < ppu.sprite_count; i++) {
//
//			//if sprite is not enabled, move on to next entry
//			if (((ppu.sprites[i].attr >> 3) & 0x1) == 0)
//				continue;
//
//			// Scanline cycle has "collided" with sprite, shifters taking over
//			if (ppu.sprites[i].x == 0) {
//				// Note Fine X scrolling does not apply to sprites, the game
//				// should maintain their relationship with the background. So
//				// we'll just use the MSB of the shifter
//
//				// Determine the pixel value...
//				uint32_t fg_pixel_lo = (ppu.ss_pattern_lo[i] & 0x80) > 0 ? 1U : 0;
//				uint32_t fg_pixel_hi = (ppu.ss_pattern_hi[i] & 0x80) > 0 ? 1U : 0;
//				fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;
//
//				// Extract the palette from the bottom two bits. Recall
//				// that foreground palettes are the latter 4 in the 
//				// palette memory.
//				fg_palette = (ppu.sprites[i].attr & 0x07) + 0x08;
//				fg_priority = (ppu.sprites[i].attr >> 4) & 0x03;
//				//fg_priority = (spriteScanlines[i].Attribute & 0x20) == 0 ? 1U : 0;
//
//				// If pixel is not transparent, we render it, and dont
//				// bother checking the rest because the earlier sprites
//				// in the list are higher priority
//				if (fg_pixel != 0)
//				{
//					if (i == 0) // Is this sprite zero?
//					{
//						ppu.sprite_zero_being_rendered = true;
//					}
//
//					break;
//				}
//			}
//		}
//	}
//
//	uint32_t finalPixel = 0x00;   // The FINAL Pixel...
//	uint32_t finalPalette = 0x00; // The FINAL Palette...
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_ENABLE) && fg_pixel > 0 && fg_priority == 0) {
//		finalPixel = fg_pixel;
//		finalPalette = fg_palette;
//	}
//	else if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE) && ppu.backgrounds[0].pixel > 0)
//	{
//		finalPixel = ppu.backgrounds[0].pixel;
//		finalPalette = ppu.backgrounds[0].palette;
//	}
//	else if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_ENABLE) && fg_pixel > 0 && fg_priority == 1)
//	{
//		finalPixel = fg_pixel;
//		finalPalette = fg_palette;
//	}
//	else if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE) && ppu.backgrounds[1].pixel > 0)
//	{
//		finalPixel = ppu.backgrounds[1].pixel;
//		finalPalette = ppu.backgrounds[1].palette;
//	}
//	else if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_ENABLE) && fg_pixel > 0 && fg_priority == 2)
//	{
//		finalPixel = fg_pixel;
//		finalPalette = fg_palette;
//	}
//
//	if (ppu.backgrounds[0].pixel > 0 && fg_pixel > 0) {
//		if (ppu.sprite_zero_hit_possible && ppu.sprite_zero_being_rendered) {
//			// Sprite zero is a collision between foreground and background
//			// so they must both be enabled
//			if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE) && CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_ENABLE)) {
//				// The left edge of the screen has specific switches to control
//				// its appearance. This is used to smooth inconsistencies when
//				// scrolling (since sprites x coord must be >= 0)
//				if (!(CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_LEFT_COL) || CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_LEFT_COL))) {
//					if (ppu.cycle >= 9 && ppu.cycle < 256) {
//						SET_BIT(ppu.registers[PPU_REG_STATUS], 4);
//					}
//				}
//				else {
//					if (ppu.cycle >= 1 && ppu.cycle < 256) {
//						SET_BIT(ppu.registers[PPU_REG_STATUS], 4);
//					}
//				}
//			}
//		}
//	}
//
//	// load color from palette
//	if (ppu.cycle > 0 && ppu.cycle <= 256 && ppu.scanline > 0 && ppu.scanline < 240) {
//		if ((ppu.cycle <= 8 && !CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_LEFT_COL)))
//		{
//			framebuffer[ppu.cycle - 1 + ppu.scanline * 256] = palette_screen[ppu.vram[0xF000]];
//		}
//		else
//		{
//			framebuffer[ppu.cycle - 1 + ppu.scanline * 256] = palette_screen[ppu.vram[0xF000 + (finalPalette << 2) + finalPixel]];
//		}
//	}
//}
//
//bool ppu_render(void) {
//	SDL_UpdateTexture(sdlTexture, NULL, framebuffer, SCREEN_WIDTH * 4);
//	SDL_RenderClear(renderer);
//	SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
//
//	SDL_RenderPresent(renderer);
//
//	SDL_Event event;
//	static bool cmd_down = false;
//	bool mouse_changed = false;
//	while (SDL_PollEvent(&event)) {
//		if (event.type == SDL_QUIT) {
//			return false;
//		}
//		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
//			return false;
//		}
//		if (event.type == SDL_KEYDOWN) {
//			bool consumed = false;
//			//if (cmd_down && !(disable_emu_cmd_keys || mouse_grabbed)) {
//			if (cmd_down && !(disable_emu_cmd_keys)) {
//				if (event.key.keysym.sym == SDLK_s) {
//					//machine_dump("user keyboard request");
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_r) {
//					//machine_reset();
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_BACKSPACE) {
//					//machine_nmi();
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_v) {
//					//machine_paste(SDL_GetClipboardText());
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_f || event.key.keysym.sym == SDLK_RETURN) {
//					is_fullscreen = !is_fullscreen;
//					SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
//					//machine_toggle_warp();
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_a) {
//					//sdcard_attach();
//					consumed = true;
//				}
//				else if (event.key.keysym.sym == SDLK_d) {
//					//sdcard_detach();
//					consumed = true;
//				}
//			}
//			if (cmd_down) {
//				if (event.key.keysym.sym == SDLK_m) {
//					//mousegrab_toggle();
//					consumed = true;
//				}
//			}
//			if (!consumed) {
//				if (event.key.keysym.scancode == LSHORTCUT_KEY || event.key.keysym.scancode == RSHORTCUT_KEY) {
//					cmd_down = true;
//				}
//				//handle_keyboard(true, event.key.keysym.sym, event.key.keysym.scancode);
//			}
//			continue;
//		}
//		if (event.type == SDL_KEYUP) {
//			if (event.key.keysym.scancode == LSHORTCUT_KEY || event.key.keysym.scancode == RSHORTCUT_KEY) {
//				cmd_down = false;
//			}
//			//handle_keyboard(false, event.key.keysym.sym, event.key.keysym.scancode);
//			continue;
//		}
//		if (event.type == SDL_MOUSEBUTTONDOWN) {
//			switch (event.button.button) {
//			case SDL_BUTTON_LEFT:
//				//mouse_button_down(0);
//				mouse_changed = true;
//				break;
//			case SDL_BUTTON_RIGHT:
//				//mouse_button_down(1);
//				mouse_changed = true;
//				break;
//			case SDL_BUTTON_MIDDLE:
//				//mouse_button_down(2);
//				mouse_changed = true;
//				break;
//			}
//		}
//		if (event.type == SDL_MOUSEBUTTONUP) {
//			switch (event.button.button) {
//			case SDL_BUTTON_LEFT:
//				//mouse_button_up(0);
//				mouse_changed = true;
//				break;
//			case SDL_BUTTON_RIGHT:
//				//mouse_button_up(1);
//				mouse_changed = true;
//				break;
//			case SDL_BUTTON_MIDDLE:
//				//mouse_button_up(2);
//				mouse_changed = true;
//				break;
//			}
//		}
//		if (event.type == SDL_MOUSEMOTION) {
//			static int mouse_x;
//			static int mouse_y;
//			//if (mouse_grabbed) {
//			//	//mouse_move(event.motion.xrel, event.motion.yrel);
//			//}
//			//else {
//			//	//mouse_move(event.motion.x - mouse_x, event.motion.y - mouse_y);
//			//}
//			mouse_x = event.motion.x;
//			mouse_y = event.motion.y;
//			mouse_changed = true;
//		}
//		if (event.type == SDL_MOUSEWHEEL) {
//			//mouse_set_wheel(event.wheel.y);
//			mouse_changed = true;
//		}
//		/*if (event.type == SDL_JOYDEVICEADDED) {
//			joystick_add(event.jdevice.which);
//		}
//		if (event.type == SDL_JOYDEVICEREMOVED) {
//			joystick_remove(event.jdevice.which);
//		}
//		if (event.type == SDL_CONTROLLERBUTTONDOWN) {
//			joystick_button_down(event.cbutton.which, event.cbutton.button);
//		}
//		if (event.type == SDL_CONTROLLERBUTTONUP) {
//			joystick_button_up(event.cbutton.which, event.cbutton.button);
//		}*/
//	}
//	if (mouse_changed) {
//		//mouse_send_state();
//	}
//	return true;
//
//	return true;
//}
//
//static uint8_t ppu_read(uint16_t portnum) {
//	uint8_t data = 0;
//
//	switch (portnum & 0x03) {
//	case PPU_VRAM_ADDR_LO:
//		//ppu.addr_latch = 0;
//		break;
//
//	case PPU_VRAM_ADDR_HI:
//		data = ppu.vram[ppu.vram_addr];
//		//ppu.vram_addr += ((ppu.reg_control >> 4) & 1) ? 32 : 1;
//		break;
//
//	case PPU_DATA:
//		if (ppu.addr_reg_flag == 0) {
//			data = ppu_vram_read(ppu.vram_addr);
//			ppu.vram_addr += ((ppu.registers[PPU_REG_CONTROL] >> 4) & 1) ? 32 : 1;
//		}
//		else {
//			data = ppu.registers[ppu.current_reg];
//		}
//		break;
//
//	case PPU_REGISTER_NUM:
//		//data = ppu.registers[ppu.current_reg];
//		break;
//	}
//
//	return data;
//}
//
//void process_register_write(uint8_t reg) {
//	switch (reg) {
//	case PPU_REG_NAMETABLE:
//		ppu.backgrounds[0].loopy_t.nametable_x = CHECK_BIT(ppu.registers[reg], 0);
//		ppu.backgrounds[0].loopy_t.nametable_y = CHECK_BIT(ppu.registers[reg], 1);
//		ppu.backgrounds[1].loopy_t.nametable_x = CHECK_BIT(ppu.registers[reg], 2);
//		ppu.backgrounds[1].loopy_t.nametable_y = CHECK_BIT(ppu.registers[reg], 3);
//		break;
//	case PPU_REG_BG0_SCROLL_X:
//		ppu.backgrounds[0].fine_x = ppu.registers[reg] & 0x07;
//		ppu.backgrounds[0].loopy_t.coarse_x = ppu.registers[reg] >> 3;
//		break;
//	case PPU_REG_BG0_SCROLL_Y:
//		ppu.backgrounds[0].loopy_t.fine_y = ppu.registers[reg] & 0x07;
//		ppu.backgrounds[0].loopy_t.coarse_y = ppu.registers[reg] >> 3;
//		break;
//	case PPU_REG_BG1_SCROLL_X:
//		ppu.backgrounds[1].fine_x = ppu.registers[reg] & 0x07;
//		ppu.backgrounds[1].loopy_t.coarse_x = ppu.registers[reg] >> 3;
//		break;
//	case PPU_REG_BG1_SCROLL_Y:
//		ppu.backgrounds[1].loopy_t.fine_y = ppu.registers[reg] & 0x07;
//		ppu.backgrounds[1].loopy_t.coarse_y = ppu.registers[reg] >> 3;
//		break;
//	}
//}
//
//static void ppu_write(uint16_t portnum, uint8_t value) {
//	switch (portnum & 0x03) {
//	case PPU_VRAM_ADDR_LO:
//		ppu.vram_addr = (ppu.vram_addr & 0xFF00) | value;
//		ppu.addr_reg_flag = 0;
//		break;
//
//	case PPU_VRAM_ADDR_HI:
//		ppu.vram_addr = ((value & 0xFF) << 8) | (ppu.vram_addr & 0x00FF);
//		ppu.addr_reg_flag = 0;
//		break;
//
//	case PPU_DATA:
//		if (ppu.addr_reg_flag == 0) {
//			ppu_vram_write(ppu.vram_addr, value);
//			ppu.vram_addr += ((ppu.registers[PPU_REG_CONTROL] >> 4) & 1) ? 32 : 1;
//		}
//		else {
//			ppu.registers[ppu.current_reg] = value;
//			process_register_write(ppu.current_reg);
//		}
//		break;
//
//	case PPU_REGISTER_NUM:
//		ppu.current_reg = value;
//		ppu.addr_reg_flag = 1;
//		break;
//	}
//}
//
//
//
//uint8_t flip_byte(uint8_t b) {
//	b = ((b & 0xF0) >> 4 | (b & 0x0F) << 4);
//	b = ((b & 0xCC) >> 2 | (b & 0x33) << 2);
//	b = ((b & 0xAA) >> 1 | (b & 0x55) << 1);
//	return b;
//}
//
//void update_shifters(void) {
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE)) {
//		ppu.backgrounds[0].shifter_pattern_lo <<= 1;
//		ppu.backgrounds[0].shifter_pattern_hi <<= 1;
//		ppu.backgrounds[0].shifter_attr_lo <<= 1;
//		ppu.backgrounds[0].shifter_attr_hi <<= 1;
//	}
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE)) {
//		ppu.backgrounds[1].shifter_pattern_lo <<= 1;
//		ppu.backgrounds[1].shifter_pattern_hi <<= 1;
//		ppu.backgrounds[1].shifter_attr_lo <<= 1;
//		ppu.backgrounds[1].shifter_attr_hi <<= 1;
//	}
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_SPR_ENABLE) && ppu.cycle >= 1 && ppu.cycle <= 258) {
//		for (int i = 0; i < ppu.sprite_count; i++) {
//			if (ppu.sprites[i].x > 0) {
//				ppu.sprites[i].x--;
//			}
//			else {
//				ppu.ss_pattern_lo[i] <<= 1;
//				ppu.ss_pattern_hi[i] <<= 1;
//			}
//		}
//	}
//}
//
//void load_background_shifters(void) {
//
//	for (int i = 0; i < 2; i++) {
//		// Each PPU update we calculate one pixel. These shifters shift 1 bit along
//		// feeding the pixel compositor with the binary information it needs. Its
//		// 16 bits wide, because the top 8 bits are the current 8 pixels being drawn
//		// and the bottom 8 bits are the next 8 pixels to be drawn. Naturally this means
//		// the required bit is always the MSB of the shifter. However, "fine x" scrolling
//		// plays a part in this too, whcih is seen later, so in fact we can choose
//		// any one of the top 8 bits.
//
//		ppu.backgrounds[i].shifter_pattern_lo = (ppu.backgrounds[i].shifter_pattern_lo & 0xFF00) | ppu.backgrounds[i].next_tile_lsb;
//		ppu.backgrounds[i].shifter_pattern_hi = (ppu.backgrounds[i].shifter_pattern_hi & 0xFF00) | ppu.backgrounds[i].next_tile_msb;
//
//		// Attribute bits do not change per pixel, rather they change every 8 pixels
//		// but are synchronised with the pattern shifters for convenience, so here
//		// we take the bottom 2 bits of the attribute word which represent which 
//		// palette is being used for the current 8 pixels and the next 8 pixels, and 
//		// "inflate" them to 8 bit words
//		ppu.backgrounds[i].shifter_attr_lo = ((ppu.backgrounds[i].shifter_attr_lo & 0xFF00) | (((ppu.backgrounds[i].next_tile_attr & 0b01) == 0b01) ? 0xFF : 0x00));
//		ppu.backgrounds[i].shifter_attr_hi = ((ppu.backgrounds[i].shifter_attr_hi & 0xFF00) | (((ppu.backgrounds[i].next_tile_attr & 0b10) == 0b10) ? 0xFF : 0x00));
//		//ppu.backgrounds[i].shifter_attr_lo = ((ppu.backgrounds[i].shifter_attr_lo & 0xFF00) | (((ppu.backgrounds[i].next_tile_attr & 0b01) == 0b01) ? 0xFF : 0x00));
//		//ppu.backgrounds[i].shifter_attr_hi = ((ppu.backgrounds[i].shifter_attr_hi & 0xFF00) | (((ppu.backgrounds[i].next_tile_attr & 0b10) == 0b10) ? 0xFF : 0x00));
//	}
//}
//
//// ==============================================================================
//// Increment x scroll
//void inc_scroll_x(void) {
//	// Note: pixel perfect scrolling horizontally is handled by the 
//	// data shifters. Here we are operating in the spatial domain of 
//	// tiles, 8x8 pixel blocks.
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE)) {
//		if (ppu.backgrounds[0].loopy_v.coarse_x == 31) {
//			ppu.backgrounds[0].loopy_v.coarse_x = 0;
//			ppu.backgrounds[0].loopy_v.nametable_x ^= 1;
//		}
//		else {
//			ppu.backgrounds[0].loopy_v.coarse_x++;
//		}
//	}
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE)) {
//		if (ppu.backgrounds[1].loopy_v.coarse_x == 31) {
//			ppu.backgrounds[1].loopy_v.coarse_x = 0;
//			ppu.backgrounds[1].loopy_v.nametable_x ^= 1;
//		}
//		else {
//			ppu.backgrounds[1].loopy_v.coarse_x++;
//		}
//	}
//}
//
//// ==============================================================================
//// Increment y scroll
//void inc_scroll_y(void) {
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE)) {
//		if (ppu.backgrounds[0].loopy_v.fine_y < 7) {
//			ppu.backgrounds[0].loopy_v.fine_y++;
//		}
//		else {
//			ppu.backgrounds[0].loopy_v.fine_y = 0;
//
//			if (ppu.backgrounds[0].loopy_v.coarse_y == 31) {
//				ppu.backgrounds[0].loopy_v.coarse_y = 0;
//				ppu.backgrounds[0].loopy_v.nametable_y ^= 1;
//			}
//			else {
//				ppu.backgrounds[0].loopy_v.coarse_y++;
//			}
//		}
//	}
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE)) {
//		if (ppu.backgrounds[1].loopy_v.fine_y < 7) {
//			ppu.backgrounds[1].loopy_v.fine_y++;
//		}
//		else {
//			ppu.backgrounds[1].loopy_v.fine_y = 0;
//
//			if (ppu.backgrounds[1].loopy_v.coarse_y == 31) {
//				ppu.backgrounds[1].loopy_v.coarse_y = 0;
//				ppu.backgrounds[1].loopy_v.nametable_y ^= 1;
//			}
//			else {
//				ppu.backgrounds[1].loopy_v.coarse_y++;
//			}
//		}
//	}
//}
//
//// ==============================================================================
//// Transfer the temporarily stored horizontal nametable access information
//// into the "pointer". Note that fine x scrolling is not part of the "pointer"
//// addressing mechanism
//void transfer_addr_x(void) {
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE)) {
//		ppu.backgrounds[0].loopy_v.nametable_x = ppu.backgrounds[0].loopy_t.nametable_x;
//		ppu.backgrounds[0].loopy_v.coarse_x = ppu.backgrounds[0].loopy_t.coarse_x;
//	}
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE)) {
//		ppu.backgrounds[1].loopy_v.nametable_x = ppu.backgrounds[1].loopy_t.nametable_x;
//		ppu.backgrounds[1].loopy_v.coarse_x = ppu.backgrounds[1].loopy_t.coarse_x;
//	}
//}
//
//// ==============================================================================
//// Transfer the temporarily stored vertical nametable access information
//// into the "pointer". Note that fine y scrolling is part of the "pointer"
//// addressing mechanism
//void transfer_addr_y(void) {
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG0_ENABLE)) {
//		ppu.backgrounds[0].loopy_v.fine_y = ppu.backgrounds[0].loopy_t.fine_y;
//		ppu.backgrounds[0].loopy_v.nametable_y = ppu.backgrounds[0].loopy_t.nametable_y;
//		ppu.backgrounds[0].loopy_v.coarse_y = ppu.backgrounds[0].loopy_t.coarse_y;
//	}
//
//	if (CHECK_BIT(ppu.registers[PPU_REG_MASK], PPU_MASK_BG1_ENABLE)) {
//		ppu.backgrounds[1].loopy_v.fine_y = ppu.backgrounds[1].loopy_t.fine_y;
//		ppu.backgrounds[1].loopy_v.nametable_y = ppu.backgrounds[1].loopy_t.nametable_y;
//		ppu.backgrounds[1].loopy_v.coarse_y = ppu.backgrounds[1].loopy_t.coarse_y;
//	}
//}
//
//uint16_t get_loopy(LOOPY* l) {
//	return  (l->coarse_x & 0b11111) +
//		((l->coarse_y & 0b11111) << 5) +
//		((l->nametable_x & 0b1) << 10) +
//		((l->nametable_y & 0b1) << 11) +
//		((l->fine_y & 0b111) << 12);
//}
//
//void set_loopy(LOOPY* l, uint16_t value) {
//	l->coarse_x = value & 0b11111;
//	l->coarse_y = (value >> 5) & 0b11111;
//	l->nametable_x = (value >> 10) & 0b1;
//	l->nametable_y = (value >> 11) & 0b1;
//	l->fine_y = (value >> 12) & 0b111;
//}
//
//void ppu_vram_write(uint16_t address, uint8_t value) {
//	ppu.vram[address] = value;
//}
//
//uint8_t ppu_vram_read(uint16_t address) {
//	return ppu.vram[address];
//}
//
//// ==============================================================================
//// PPU load default charset
//bool ppu_load_default_font(uint16_t addr) {
//
//	int length = sizeof(default_ppu_font) / sizeof(default_ppu_font[0]);
//
//	uint16_t a = addr;
//	for (int c = 0; c < 128; c++) {
//
//		for (int i = 0; i < 8; i++) {
//			ppu_vram_write(a, default_ppu_font[c * 8 + i]);
//			a++;
//		}
//
//		for (int i = 0; i < 8; i++) {
//			ppu_vram_write(a, 0);
//			a++;
//		}
//	}
//
//	return true;
//}
//
//// ==============================================================================
//// PPU load BIN file
//bool ppu_load_bin(char* filename, uint16_t addr) {
//
//	FILE* file = fopen(filename, "rb");
//	if (file == NULL)
//		return false;
//	fseek(file, 0L, SEEK_END);
//	uint32_t readsize = ftell(file);
//	rewind(file);
//
//	uint8_t* tiles = (uint8_t*)malloc(sizeof(uint8_t) * readsize);
//	if (tiles == NULL) {
//		fclose(file);
//		return false;
//	}
//
//	fread(tiles, sizeof(uint8_t) * readsize, 1, file);
//
//	if (readsize <= 0) {
//		fclose(file);
//		return false;
//	}
//
//	memcpy(ppu.vram + addr, tiles, readsize);
//	printf("BIN data %s loaded at 0x%04X (%d KB)\n", filename, addr, readsize >> 10);
//	free(tiles);
//	fclose(file);
//	return true;
//}
//
//void ppu_load_test_sprites(void) {
//
//}
//
//void ppu_write_register(uint8_t reg, uint8_t value) {
//	ppu_write(0x03, reg);
//	ppu_write(0x02, value);
//}
//
//void ppu_load_test_data(void) {
//
//	ppu.vram[0xF000] = 0x3D;
//	ppu.vram[0xF001] = 0x00;
//	ppu.vram[0xF002] = 0x01;
//	ppu.vram[0xF003] = 0x02;
//
//	ppu.vram[0x8001] = 0x61;
//
//	ppu_write_register(PPU_REG_PATTERN, 0b00000000);
//	ppu_write_register(PPU_REG_MASK, 0b00001001);
//	ppu_write_register(PPU_REG_BG0_SCROLL_X, 0x00);
//	
//}
//
