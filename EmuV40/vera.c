#include "vera.h"
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "ports.h"
#include "joystick.h"
#include "kb.h"
#include "cpu.h"
#include "machine.h"

static bool is_fullscreen = false;
bool mouse_grabbed = false;
bool no_keyboard_capture = false;
bool kernal_mouse_enabled = false;

static const uint8_t vera_version_string[] = { 'V',
	VERA_VERSION_MAJOR,
	VERA_VERSION_MINOR,
	VERA_VERSION_PATCH
};



static const uint16_t default_palette[] = {
	0x000,0xFFF,0x800,0xafe,0xc4c,0x0c5,0x00a,0xee7,0xd85,0x640,0xf77,0x333,0x777,0xaf6,0x08f,0xbbb,
	0x000,0x111,0x222,0x333,0x444,0x555,0x666,0x777,0x888,0x999,0xaaa,0xbbb,0xccc,0xddd,0xeee,0xfff,
	0x211,0x433,0x644,0x866,0xa88,0xc99,0xfbb,0x211,0x422,0x633,0x844,0xa55,0xc66,0xf77,0x200,0x411,
	0x611,0x822,0xa22,0xc33,0xf33,0x200,0x400,0x600,0x800,0xa00,0xc00,0xf00,0x221,0x443,0x664,0x886,
	0xaa8,0xcc9,0xfeb,0x211,0x432,0x653,0x874,0xa95,0xcb6,0xfd7,0x210,0x431,0x651,0x862,0xa82,0xca3,
	0xfc3,0x210,0x430,0x640,0x860,0xa80,0xc90,0xfb0,0x121,0x343,0x564,0x786,0x9a8,0xbc9,0xdfb,0x121,
	0x342,0x463,0x684,0x8a5,0x9c6,0xbf7,0x120,0x241,0x461,0x582,0x6a2,0x8c3,0x9f3,0x120,0x240,0x360,
	0x480,0x5a0,0x6c0,0x7f0,0x121,0x343,0x465,0x686,0x8a8,0x9ca,0xbfc,0x121,0x242,0x364,0x485,0x5a6,
	0x6c8,0x7f9,0x020,0x141,0x162,0x283,0x2a4,0x3c5,0x3f6,0x020,0x041,0x061,0x082,0x0a2,0x0c3,0x0f3,
	0x122,0x344,0x466,0x688,0x8aa,0x9cc,0xbff,0x122,0x244,0x366,0x488,0x5aa,0x6cc,0x7ff,0x022,0x144,
	0x166,0x288,0x2aa,0x3cc,0x3ff,0x022,0x044,0x066,0x088,0x0aa,0x0cc,0x0ff,0x112,0x334,0x456,0x668,
	0x88a,0x9ac,0xbcf,0x112,0x224,0x346,0x458,0x56a,0x68c,0x79f,0x002,0x114,0x126,0x238,0x24a,0x35c,
	0x36f,0x002,0x014,0x016,0x028,0x02a,0x03c,0x03f,0x112,0x334,0x546,0x768,0x98a,0xb9c,0xdbf,0x112,
	0x324,0x436,0x648,0x85a,0x96c,0xb7f,0x102,0x214,0x416,0x528,0x62a,0x83c,0x93f,0x102,0x204,0x306,
	0x408,0x50a,0x60c,0x70f,0x212,0x434,0x646,0x868,0xa8a,0xc9c,0xfbe,0x211,0x423,0x635,0x847,0xa59,
	0xc6b,0xf7d,0x201,0x413,0x615,0x826,0xa28,0xc3a,0xf3c,0x201,0x403,0x604,0x806,0xa08,0xc09,0xf0b
};

static uint8_t default_font[] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		//   0  nul
			0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,		//   1  so
			0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,		//   2  stx
			0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,		//   3  etx
			0x10, 0x38, 0x7C, 0xFE,	0x7C, 0x38, 0x10, 0x00,		//   4  eot
			0x38, 0x7C, 0x38, 0xFE,	0xFE, 0x7C, 0x38, 0x7C,		//   5  enq
			0x10, 0x10, 0x38, 0x7C,	0xFE, 0x7C, 0x38, 0x7C,		//   6  ack
			0x00, 0x00, 0x18, 0x3C,	0x3C, 0x18, 0x00, 0x00,		//   7  bel
			0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,		//   8  bs
			0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,		//   9  t
			0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,		//  10  lf
			0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,		//  11  vt
			0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,		//  12  ff
			0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,		//  13  cr
			0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,		//  14  so
			0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,		//  15  si
			0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,		//  16  dle
			0x02, 0x0E, 0x3E, 0xFE,	0x3E, 0x0E, 0x02, 0x00,		//  17  dc1
			0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18,		//  18  dc2
			0x66, 0x66, 0x66, 0x66,	0x66, 0x00, 0x66, 0x00,		//  19  dc3
			0x7F, 0xDB, 0xDB, 0x7B,	0x1B, 0x1B, 0x1B, 0x00,		//  20x  dc4
			0x3E, 0x63, 0x38, 0x6C,	0x6C, 0x38, 0xCC, 0x78,		//  21  nak
			0x00, 0x00, 0x00, 0x00,	0x7E, 0x7E, 0x7E, 0x00,		//  22  syn
			0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,		//  23  etb
			0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,		//  24  can
			0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,		//  25  em
			0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,		//  26  sub
			0x00, 0x30, 0x60, 0xFE,	0x60, 0x30, 0x00, 0x00,		//  27  esc
			0x00, 0x00, 0xC0, 0xC0,	0xC0, 0xFE, 0x00, 0x00,		//  28  fs
			0x00, 0x24, 0x66, 0xFF,	0x66, 0x24, 0x00, 0x00,		//  29  gs
			0x00, 0x18, 0x3C, 0x7E,	0xFF, 0xFF, 0x00, 0x00,		//  30  rs
			0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00,		//  31  us
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		//  32  space
			0x30, 0x78, 0x78, 0x30,	0x30, 0x00, 0x30, 0x00,		//  33  !
			0x6C, 0x6C, 0x6C, 0x00,	0x00, 0x00, 0x00, 0x00,		//  34  "
			0x6C, 0x6C, 0xFE, 0x6C,	0xFE, 0x6C, 0x6C, 0x00,		//  35  #
			0x30, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00,		//  36  $
			0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00,		//  37  %
			0x38, 0x6C, 0x38, 0x76,	0xDC, 0xCC, 0x76, 0x00,		//  38  &
			0x60, 0x60, 0xC0, 0x00,	0x00, 0x00, 0x00, 0x00,		//  39  '
			0x18, 0x30, 0x60, 0x60,	0x60, 0x30, 0x18, 0x00,		//  40  (
			0x60, 0x30, 0x18, 0x18,	0x18, 0x30, 0x60, 0x00,		//  41  )
			0x00, 0x66, 0x3C, 0xFF,	0x3C, 0x66, 0x00, 0x00,		//  42  *
			0x00, 0x30, 0x30, 0xFC,	0x30, 0x30, 0x00, 0x00,		//  43  +
			0x00, 0x00, 0x00, 0x00,	0x00, 0x30, 0x30, 0x60,		//  44  ,
			0x00, 0x00, 0x00, 0xFC,	0x00, 0x00, 0x00, 0x00,		//  45  -
			0x00, 0x00, 0x00, 0x00,	0x00, 0x30, 0x30, 0x00,		//  46  .
			0x06, 0x0C, 0x18, 0x30,	0x60, 0xC0, 0x80, 0x00,		//  47  /
			0x7C, 0xC6, 0xCE, 0xDE,	0xF6, 0xE6, 0x7C, 0x00,		//  48  0
			0x30, 0x70, 0x30, 0x30,	0x30, 0x30, 0xFC, 0x00,		//  49  1
			0x78, 0xCC, 0x0C, 0x38,	0x60, 0xCC, 0xFC, 0x00,		//  50  2
			0x78, 0xCC, 0x0C, 0x38,	0x0C, 0xCC, 0x78, 0x00,		//  51  3
			0x1C, 0x3C, 0x6C, 0xCC,	0xFE, 0x0C, 0x1E, 0x00,		//  52  4
			0xFC, 0xC0, 0xF8, 0x0C,	0x0C, 0xCC, 0x78, 0x00,		//  53  5
			0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00,		//  54  6
			0xFC, 0xCC, 0x0C, 0x18,	0x30, 0x30, 0x30, 0x00,		//  55  7
			0x78, 0xCC, 0xCC, 0x78,	0xCC, 0xCC, 0x78, 0x00,		//  56  8
			0x78, 0xCC, 0xCC, 0x7C,	0x0C, 0x18, 0x70, 0x00,		//  57  9
			0x00, 0x30, 0x30, 0x00,	0x00, 0x30, 0x30, 0x00,		//  58  :
			0x00, 0x30, 0x30, 0x00,	0x00, 0x30, 0x30, 0x60,		//  59  //
			0x18, 0x30, 0x60, 0xC0,	0x60, 0x30, 0x18, 0x00,		//  60x  <
			0x00, 0x00, 0xFC, 0x00,	0x00, 0xFC, 0x00, 0x00,		//  61  =
			0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,		//  62  >
			0x78, 0xCC, 0x0C, 0x18,	0x30, 0x00, 0x30, 0x00,		//  63  ?
			0x7C, 0xC6, 0xDE, 0xDE,	0xDE, 0xC0, 0x78, 0x00,		//  64  @
			0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00,		//  65  A
			0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,		//  66  B
			0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00,		//  67  C
			0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00,		//  68  D
			0xFE, 0x62, 0x68, 0x78,	0x68, 0x62, 0xFE, 0x00,		//  69  E
			0xFE, 0x62, 0x68, 0x78,	0x68, 0x60, 0xF0, 0x00,		//  70x  F
			0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00,		//  71  G
			0xCC, 0xCC, 0xCC, 0xFC,	0xCC, 0xCC, 0xCC, 0x00,		//  72  
			0x78, 0x30, 0x30, 0x30,	0x30, 0x30, 0x78, 0x00,		//  73  I
			0x1E, 0x0C, 0x0C, 0x0C,	0xCC, 0xCC, 0x78, 0x00,		//  74  J
			0xE6, 0x66, 0x6C, 0x78,	0x6C, 0x66, 0xE6, 0x00,		//  75  K
			0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00,		//  76  L
			0xC6, 0xEE, 0xFE, 0xFE,	0xD6, 0xC6, 0xC6, 0x00,		//  77  M
			0xC6, 0xE6, 0xF6, 0xDE,	0xCE, 0xC6, 0xC6, 0x00,		//  78  N
			0x38, 0x6C, 0xC6, 0xC6,	0xC6, 0x6C, 0x38, 0x00,		//  79  O
			0xFC, 0x66, 0x66, 0x7C,	0x60, 0x60, 0xF0, 0x00,		//  80x  P
			0x78, 0xCC, 0xCC, 0xCC,	0xDC, 0x78, 0x1C, 0x00,		//  81  Q
			0xFC, 0x66, 0x66, 0x7C,	0x6C, 0x66, 0xE6, 0x00,		//  82  R
			0x78, 0xCC, 0xE0, 0x70,	0x1C, 0xCC, 0x78, 0x00,		//  83  S
			0xFC, 0xB4, 0x30, 0x30,	0x30, 0x30, 0x78, 0x00,		//  84  T
			0xCC, 0xCC, 0xCC, 0xCC,	0xCC, 0xCC, 0xFC, 0x00,		//  85  U
			0xCC, 0xCC, 0xCC, 0xCC,	0xCC, 0x78, 0x30, 0x00,		//  86  V
			0xC6, 0xC6, 0xC6, 0xD6,	0xFE, 0xEE, 0xC6, 0x00,		//  87  W
			0xC6, 0xC6, 0x6C, 0x38,	0x38, 0x6C, 0xC6, 0x00,		//  88  X
			0xCC, 0xCC, 0xCC, 0x78,	0x30, 0x30, 0x78, 0x00,		//  89  Y
			0xFE, 0xC6, 0x8C, 0x18,	0x32, 0x66, 0xFE, 0x00,		//  90x  Z
			0x78, 0x60, 0x60, 0x60,	0x60, 0x60, 0x78, 0x00,		//  91  [
			0xC0, 0x60, 0x30, 0x18,	0x0C, 0x06, 0x02, 0x00,		//  92  backslas
			0x78, 0x18, 0x18, 0x18,	0x18, 0x18, 0x78, 0x00,		//  93  ]
			0x10, 0x38, 0x6C, 0xC6,	0x00, 0x00, 0x00, 0x00,		//  94  ^
			0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0xFF,		//  95  _
			0x30, 0x30, 0x18, 0x00,	0x00, 0x00, 0x00, 0x00,		//  96  `
			0x00, 0x00, 0x78, 0x0C,	0x7C, 0xCC, 0x76, 0x00,		//  97  a
			0xE0, 0x60, 0x60, 0x7C,	0x66, 0x66, 0xDC, 0x00,		//  98  b
			0x00, 0x00, 0x78, 0xCC,	0xC0, 0xCC, 0x78, 0x00,		//  99  c
			0x1C, 0x0C, 0x0C, 0x7C,	0xCC, 0xCC, 0x76, 0x00,		// 10x0x  d
			0x00, 0x00, 0x78, 0xCC,	0xFC, 0xC0, 0x78, 0x00,		// 10x1  e
			0x38, 0x6C, 0x60, 0xF0,	0x60, 0x60, 0xF0, 0x00,		// 10x2  f
			0x00, 0x00, 0x76, 0xCC,	0xCC, 0x7C, 0x0C, 0xF8,		// 10x3  g
			0xE0, 0x60, 0x6C, 0x76,	0x66, 0x66, 0xE6, 0x00,		// 10x4  
			0x30, 0x00, 0x70, 0x30,	0x30, 0x30, 0x78, 0x00,		// 10x5  i
			0x0C, 0x00, 0x0C, 0x0C,	0x0C, 0xCC, 0xCC, 0x78,		// 10x6  j
			0xE0, 0x60, 0x66, 0x6C,	0x78, 0x6C, 0xE6, 0x00,		// 10x7  k
			0x70, 0x30, 0x30, 0x30,	0x30, 0x30, 0x78, 0x00,		// 10x8  l
			0x00, 0x00, 0xCC, 0xFE,	0xFE, 0xD6, 0xC6, 0x00,		// 10x9  m
			0x00, 0x00, 0xF8, 0xCC,	0xCC, 0xCC, 0xCC, 0x00,		// 110x  n
			0x00, 0x00, 0x78, 0xCC,	0xCC, 0xCC, 0x78, 0x00,		// 111  o
			0x00, 0x00, 0xDC, 0x66,	0x66, 0x7C, 0x60, 0xF0,		// 112  p
			0x00, 0x00, 0x76, 0xCC,	0xCC, 0x7C, 0x0C, 0x1E,		// 113  q
			0x00, 0x00, 0xDC, 0x76,	0x66, 0x60, 0xF0, 0x00,		// 114  r
			0x00, 0x00, 0x7C, 0xC0,	0x78, 0x0C, 0xF8, 0x00,		// 115  s
			0x10, 0x30, 0x7C, 0x30,	0x30, 0x34, 0x18, 0x00,		// 116  t
			0x00, 0x00, 0xCC, 0xCC,	0xCC, 0xCC, 0x76, 0x00,		// 117  u
			0x00, 0x00, 0xCC, 0xCC,	0xCC, 0x78, 0x30, 0x00,		// 118  v
			0x00, 0x00, 0xC6, 0xD6, 0xFE, 0xFE, 0x6C, 0x00,		// 119  w
			0x00, 0x00, 0xC6, 0x6C,	0x38, 0x6C, 0xC6, 0x00,		// 120x  x
			0x00, 0x00, 0xCC, 0xCC,	0xCC, 0x7C, 0x0C, 0xF8,		// 121  y
			0x00, 0x00, 0xFC, 0x98,	0x30, 0x64, 0xFC, 0x00,		// 122  z
			0x1C, 0x30, 0x30, 0xE0,	0x30, 0x30, 0x1C, 0x00,		// 123  {
			0x18, 0x18, 0x18, 0x00,	0x18, 0x18, 0x18, 0x00,		// 124  |
			0xE0, 0x30, 0x30, 0x1C,	0x30, 0x30, 0xE0, 0x00,		// 125  }
			0x76, 0xDC, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,		// 126  ~
			0x00, 0x10, 0x38, 0x6C,	0xC6, 0xC6, 0xFE, 0x00		// 127  del
};


static const int increments[32] = {
	0,   0,
	1,   -1,
	2,   -2,
	4,   -4,
	8,   -8,
	16,  -16,
	32,  -32,
	64,  -64,
	128, -128,
	256, -256,
	512, -512,
	40,  -40,
	80,  -80,
	160, -160,
	320, -320,
	640, -640,
};

static void video_space_read_range(VERA* vera, uint8_t* dest, uint32_t address, uint32_t size);

static int calc_layer_eff_x(const struct video_layer_properties* props, const int x);
static int calc_layer_eff_y(const struct video_layer_properties* props, const int y);
static uint32_t calc_layer_map_addr_base2(const struct video_layer_properties* props, const int eff_x, const int eff_y);
static void refresh_layer_properties(VERA* vera, const uint8_t layer);
static void screenshot(void);
static void refresh_sprite_properties(VERA* vera, const uint16_t sprite);
static void refresh_palette(VERA* vera);
static void expand_4bpp_data(uint8_t* dst, const uint8_t* src, int dst_size);
static void render_sprite_line(VERA* vera, const uint16_t y);
static void render_layer_line_text(VERA* vera, uint8_t layer, uint16_t y);
static void render_layer_line_tile(VERA* vera, uint8_t layer, uint16_t y);
static void render_layer_line_bitmap(VERA* vera, uint8_t layer, uint16_t y);
static uint8_t calculate_line_col_index(uint8_t spr_zindex, uint8_t spr_col_index, uint8_t l1_col_index, uint8_t l2_col_index);
static void render_line(VERA* vera, uint16_t y, float scan_pos_x);
static void update_isr_and_coll(VERA* vera, uint16_t y, uint16_t compare);
uint32_t get_and_inc_address(VERA* vera, bool write);
static void video_space_read_range(VERA* vera, uint8_t* dest, uint32_t address, uint32_t size);
void process_register_write(VERA* vera, uint8_t reg, uint8_t value);


void mousegrab_toggle(VERA* vera) {
	mouse_grabbed = !mouse_grabbed;
	SDL_SetWindowGrab(vera->window, mouse_grabbed && !no_keyboard_capture);
	SDL_SetRelativeMouseMode(mouse_grabbed);
	SDL_ShowCursor((mouse_grabbed || kernal_mouse_enabled) ? SDL_DISABLE : SDL_ENABLE);
	sprintf(vera->window_title, WINDOW_TITLE "%s", mouse_grabbed ? MOUSE_GRAB_MSG : "");
	video_update_title(vera, vera->window_title);
}

bool vera_init(VERA* vera, int window_scale, float screen_x_scale, char* quality, bool fullscreen, float opacity) {
	/*set_port_write_redirector(0xC0, 0xDF, &vera_write);
	set_port_read_redirector(0xC0, 0xDF, &vera_read);*/

	//memset((void*)&vera, 0, sizeof(VERA));

	ports_cbRegister(0xE0, 2, (void*)vera_read, NULL, (void*)vera_write, NULL, vera);

	uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
	vera_reset(vera);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality);
	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1"); // Grabs keyboard shortcuts from the system during window grab
	SDL_CreateWindowAndRenderer((int)(SCREEN_WIDTH * window_scale * screen_x_scale), (int)(SCREEN_HEIGHT * window_scale), window_flags, &vera->window, &vera->renderer);
	SDL_SetWindowResizable(vera->window, true);
	SDL_RenderSetLogicalSize(vera->renderer, (int)(SCREEN_WIDTH * screen_x_scale), SCREEN_HEIGHT);

	vera->sdlTexture = SDL_CreateTexture(vera->renderer,
		SDL_PIXELFORMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_SetWindowTitle(vera->window, WINDOW_TITLE);
	//SDL_SetWindowIcon(window, CommanderX16Icon());
	if (fullscreen) {
		is_fullscreen = true;
		SDL_SetWindowFullscreen(vera->window, SDL_WINDOW_FULLSCREEN);
	}
	else {
		/*int winX, winY;
		SDL_GetWindowPosition(window, &winX, &winY);
		if (winX < 0 || winY < APPROX_TITLEBAR_HEIGHT) {
			winX = winX < 0 ? 0 : winX;
			winY = winY < APPROX_TITLEBAR_HEIGHT ? APPROX_TITLEBAR_HEIGHT : winY;
			SDL_SetWindowPosition(window, winX, winY);
		}*/
		SDL_SetWindowPosition(vera->window, 100, 100);
	}

	SDL_SetWindowOpacity(vera->window, opacity);

	//extern void video_win32_set_rounded_corners(SDL_Window * window);
	//video_win32_set_rounded_corners(window);

	//if (debugger_enabled) {
	//	//DEBUGInitUI(renderer);
	//}

	if (grab_mouse && !mouse_grabbed)
		mousegrab_toggle(vera);

	return true;
}

void vera_reset(VERA* vera) {
	// init I/O registers
	vera->io_addr = 0;
	vera->io_inc = 0;
	vera->io_dcsel = 0;
	vera->io_rddata = 0;
	vera->io_rddata = 0;

	vera->ien = 0;
	vera->isr = 0;
	vera->irq_line = 0;

	//init registers
	memset(vera->registers, 0, sizeof(vera->registers));
	vera->registers[REG_CTRL] = 0;
	vera->registers[REG_IEN] = 0;
	vera->registers[REG_IRQ_LINE_L] = 0;
	vera->registers[REG_HSCALE] = 128; // hscale = 1.0
	vera->registers[REG_VSCALE] = 128; // vscale = 1.0
	vera->registers[REG_BORDER_COLOR] = 0; //border color
	vera->registers[REG_HSTART] = 0; //hstart
	vera->registers[REG_HSTOP] = 640 >> 2;; //hstop
	vera->registers[REG_VSTART] = 0; //vstart
	vera->registers[REG_VSTOP] = 480 >> 1; //vstop

	vera->registers[REG_L0_CONFIG] = 0; //L0 config
	vera->registers[REG_L0_MAP_BASE] = 0; //L0 map base [15:9]
	vera->registers[REG_L0_TILE_BASE] = 0; //L0 tile base [15:11]
	vera->registers[REG_L0_HSCROLL_L] = 0; //L0 hscroll [7:0]
	vera->registers[REG_L0_HSCROLL_H] = 0; //L0 hscroll [11:8]
	vera->registers[REG_L0_VSCROLL_L] = 0; //L0 vscroll [7:0]
	vera->registers[REG_L0_VSCROLL_H] = 0; //L0 vscroll [11:8]

	vera->registers[REG_L1_CONFIG] = 0; //L1 config
	vera->registers[REG_L1_MAP_BASE] = 0; //L1 map base [15:9]
	vera->registers[REG_L1_TILE_BASE] = 0; //L1 tile base [15:11]
	vera->registers[REG_L1_HSCROLL_L] = 0; //L1 hscroll [7:0]
	vera->registers[REG_L1_HSCROLL_H] = 0; //L1 hscroll [11:8]
	vera->registers[REG_L1_VSCROLL_L] = 0; //L1 vscroll [7:0]
	vera->registers[REG_L1_VSCROLL_H] = 0; //L1 vscroll [11:8]

	vera->registers[REG_ADDR_INC] = 0;


	// init Layer registers
	memset(vera->reg_layer, 0, sizeof(vera->reg_layer));

	// init composer registers
	//memset(vera->reg_composer, 0, sizeof(vera->reg_composer));
	//vera->reg_composer[1] = 128; // hscale = 1.0
	//vera->reg_composer[2] = 128; // vscale = 1.0
	//vera->reg_composer[5] = 640 >> 2;
	//vera->reg_composer[7] = 480 >> 1;

	// init sprite data
	memset(vera->sprite_data, 0, sizeof(vera->sprite_data));

	// copy palette
	memcpy(vera->palette, default_palette, sizeof(vera->palette));
	for (int i = 0; i < 256; i++) {
		vera->palette[i * 2 + 0] = default_palette[i] & 0xff;
		vera->palette[i * 2 + 1] = default_palette[i] >> 8;
	}

	refresh_palette(vera);

	// fill video RAM with random data
	for (uint32_t i = 0; i < 0x10000; i++) {
		vera->video_ram[i] = rand();
	}

	vera->sprite_line_collisions = 0;

	vera->vga_scan_pos_x = 0;
	vera->vga_scan_pos_y = 0;
	vera->ntsc_half_cnt = 0;
	vera->ntsc_scan_pos_y = 0;
}

bool vera_step(VERA* vera, float mhz, uint32_t steps, bool midline) {
	uint16_t y = 0;
	bool ntsc_mode = false;// vera->registers[0] & 2;// vera->reg_composer[0] & 2;
	bool new_frame = false;
	vera->vga_scan_pos_x += (float)(PIXEL_FREQ * steps / mhz);
	if (vera->vga_scan_pos_x > VGA_SCAN_WIDTH) {
		vera->vga_scan_pos_x -= VGA_SCAN_WIDTH;
		if (!ntsc_mode) {
			render_line(vera, vera->vga_scan_pos_y - VGA_Y_OFFSET, VGA_SCAN_WIDTH);
		}
		vera->vga_scan_pos_y++;
		if (vera->vga_scan_pos_y == SCAN_HEIGHT) {
			vera->vga_scan_pos_y = 0;
			if (!ntsc_mode) {
				new_frame = true;
				vera->frame_count++;
			}
		}
		if (!ntsc_mode) {
			update_isr_and_coll(vera, vera->vga_scan_pos_y - VGA_Y_OFFSET, vera->irq_line);
		}
	}
	else if (midline) {
		if (!ntsc_mode) {
			render_line(vera, vera->vga_scan_pos_y - VGA_Y_OFFSET, vera->vga_scan_pos_x);
		}
	}
	vera->ntsc_half_cnt += (float)(PIXEL_FREQ * steps / mhz);
	if (vera->ntsc_half_cnt > NTSC_HALF_SCAN_WIDTH) {
		vera->ntsc_half_cnt -= NTSC_HALF_SCAN_WIDTH;
		if (ntsc_mode) {
			if (vera->ntsc_scan_pos_y < SCAN_HEIGHT) {
				y = vera->ntsc_scan_pos_y - NTSC_Y_OFFSET_LOW;
				if ((y & 1) == 0) {
					render_line(vera, y, NTSC_HALF_SCAN_WIDTH);
				}
			}
			else {
				y = vera->ntsc_scan_pos_y - NTSC_Y_OFFSET_HIGH;
				if ((y & 1) == 0) {
					render_line(vera, y | 1, NTSC_HALF_SCAN_WIDTH);
				}
			}
		}
		vera->ntsc_scan_pos_y++;
		if (vera->ntsc_scan_pos_y == SCAN_HEIGHT) {
			vera->registers[0] |= 0x80;
			if (ntsc_mode) {
				new_frame = true;
				vera->frame_count++;
			}
		}
		if (vera->ntsc_scan_pos_y == SCAN_HEIGHT * 2) {
			vera->registers[0] &= ~0x80;
			vera->ntsc_scan_pos_y = 0;
			if (ntsc_mode) {
				new_frame = true;
				vera->frame_count++;
			}
		}
		if (ntsc_mode) {
			// this is correct enough for even screen heights
			if (vera->ntsc_scan_pos_y < SCAN_HEIGHT) {
				update_isr_and_coll(vera, vera->ntsc_scan_pos_y - NTSC_Y_OFFSET_LOW, vera->irq_line & ~1);
			}
			else {
				update_isr_and_coll(vera, vera->ntsc_scan_pos_y - NTSC_Y_OFFSET_HIGH, vera->irq_line & ~1);
			}
		}
	}
	else if (midline) {
		if (ntsc_mode) {
			if (vera->ntsc_scan_pos_y < SCAN_HEIGHT) {
				y = vera->ntsc_scan_pos_y - NTSC_Y_OFFSET_LOW;
				if ((y & 1) == 0) {
					render_line(vera, y, vera->ntsc_half_cnt);
				}
			}
			else {
				y = vera->ntsc_scan_pos_y - NTSC_Y_OFFSET_HIGH;
				if ((y & 1) == 0) {
					render_line(vera, y | 1, vera->ntsc_half_cnt);
				}
			}
		}
	}

	return new_frame;
}

bool vera_update(VERA* vera) {
	static bool cmd_down = false;
	bool mouse_changed = false;

	// for activity LED, overlay red 8x4 square into top right of framebuffer
	// for progressive modes, draw LED only on even scanlines
	/*for (int y = 0; y < 4; y += 1 + !!((reg_composer[0] & 0x0b) > 0x09)) {
		for (int x = SCREEN_WIDTH - 8; x < SCREEN_WIDTH; x++) {
			uint8_t b = framebuffer[(y * SCREEN_WIDTH + x) * 4 + 0];
			uint8_t g = framebuffer[(y * SCREEN_WIDTH + x) * 4 + 1];
			uint8_t r = framebuffer[(y * SCREEN_WIDTH + x) * 4 + 2];
			r = (uint32_t)r * (255 - activity_led) / 255 + activity_led;
			g = (uint32_t)g * (255 - activity_led) / 255;
			b = (uint32_t)b * (255 - activity_led) / 255;
			framebuffer[(y * SCREEN_WIDTH + x) * 4 + 0] = b;
			framebuffer[(y * SCREEN_WIDTH + x) * 4 + 1] = g;
			framebuffer[(y * SCREEN_WIDTH + x) * 4 + 2] = r;
			framebuffer[(y * SCREEN_WIDTH + x) * 4 + 3] = 0x00;
		}
	}*/

	SDL_UpdateTexture(vera->sdlTexture, NULL, vera->framebuffer, SCREEN_WIDTH * 4);

	//if (record_gif > RECORD_GIF_PAUSED) {
	//	if (!GifWriteFrame(&gif_writer, framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT, 2, 8, false)) {
	//		// if that failed, stop recording
	//		GifEnd(&gif_writer);
	//		record_gif = RECORD_GIF_DISABLED;
	//		printf("Unexpected end of recording.\n");
	//	}
	//	if (record_gif == RECORD_GIF_SINGLE) { // if single-shot stop recording
	//		record_gif = RECORD_GIF_PAUSED;  // need to close in video_end()
	//	}
	//}

	SDL_RenderClear(vera->renderer);
	SDL_RenderCopy(vera->renderer, vera->sdlTexture, NULL, NULL);

	/*if (debugger_enabled && showDebugOnRender != 0) {
		DEBUGRenderDisplay(SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_RenderPresent(renderer);
		return true;
	}*/

	SDL_RenderPresent(vera->renderer);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return false;
		}
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
			return false;
		}
		if (event.type == SDL_KEYDOWN) {
			bool consumed = false;
			if (cmd_down && !(disable_emu_cmd_keys || mouse_grabbed)) {
				if (event.key.keysym.sym == SDLK_s) {
					//machine_dump("user keyboard request");
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_r) {
					hard_reset = true;
					reset_requested = true;
					//machine_reset();
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_BACKSPACE) {
					//machine_nmi();
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_v) {
					//machine_paste(SDL_GetClipboardText());
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_f || event.key.keysym.sym == SDLK_RETURN) {
					is_fullscreen = !is_fullscreen;
					SDL_SetWindowFullscreen(vera->window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
					//machine_toggle_warp();
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_a) {
					//sdcard_attach();
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_d) {
					//sdcard_detach();
					consumed = true;
				}
				else if (event.key.keysym.sym == SDLK_p) {
					screenshot();
					consumed = true;
				}
			}
			if (cmd_down) {
				if (event.key.keysym.sym == SDLK_m) {
					mousegrab_toggle(vera);
					consumed = true;
				}
			}
			if (!consumed) {
				if (event.key.keysym.scancode == LSHORTCUT_KEY || event.key.keysym.scancode == RSHORTCUT_KEY) {
					cmd_down = true;
				}
				handle_keyboard(true, event.key.keysym.sym, event.key.keysym.scancode);
			}
			continue;
		}
		if (event.type == SDL_KEYUP) {
			if (event.key.keysym.scancode == LSHORTCUT_KEY || event.key.keysym.scancode == RSHORTCUT_KEY) {
				cmd_down = false;
			}
			handle_keyboard(false, event.key.keysym.sym, event.key.keysym.scancode);
			continue;
		}
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				//mouse_button_down(0);
				mouse_changed = true;
				break;
			case SDL_BUTTON_RIGHT:
				//mouse_button_down(1);
				mouse_changed = true;
				break;
			case SDL_BUTTON_MIDDLE:
				//mouse_button_down(2);
				mouse_changed = true;
				break;
			}
		}
		if (event.type == SDL_MOUSEBUTTONUP) {
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				//mouse_button_up(0);
				mouse_changed = true;
				break;
			case SDL_BUTTON_RIGHT:
				//mouse_button_up(1);
				mouse_changed = true;
				break;
			case SDL_BUTTON_MIDDLE:
				//mouse_button_up(2);
				mouse_changed = true;
				break;
			}
		}
		if (event.type == SDL_MOUSEMOTION) {
			static int mouse_x;
			static int mouse_y;
			/*if (mouse_grabbed) {
				mouse_move(event.motion.xrel, event.motion.yrel);
			}
			else {
				mouse_move(event.motion.x - mouse_x, event.motion.y - mouse_y);
			}*/
			mouse_x = event.motion.x;
			mouse_y = event.motion.y;
			mouse_changed = true;
		}
		if (event.type == SDL_MOUSEWHEEL) {
			//mouse_set_wheel(event.wheel.y);
			mouse_changed = true;
		}

		if (event.type == SDL_JOYDEVICEADDED) {
			joystick_add(event.jdevice.which);
		}
		if (event.type == SDL_JOYDEVICEREMOVED) {
			joystick_remove(event.jdevice.which);
		}
		if (event.type == SDL_CONTROLLERBUTTONDOWN) {
			joystick_button_down(event.cbutton.which, event.cbutton.button);
		}
		if (event.type == SDL_CONTROLLERBUTTONUP) {
			joystick_button_up(event.cbutton.which, event.cbutton.button);
		}

	}
	if (mouse_changed) {
		//mouse_send_state();
	}
	return true;
}

void vera_end(VERA* vera) {
	/*if (debugger_enabled) {
		DEBUGFreeUI();
	}*/

	/*if (record_gif != RECORD_GIF_DISABLED) {
		GifEnd(&gif_writer);
		record_gif = RECORD_GIF_DISABLED;
	}*/

	is_fullscreen = false;
	SDL_SetWindowFullscreen(vera->window, 0);
	SDL_DestroyRenderer(vera->renderer);
	SDL_DestroyWindow(vera->window);
}

bool vera_get_irq_out(VERA* vera) {
	uint8_t tmp_isr = vera->isr;
	return (tmp_isr & vera->ien) != 0;
}

void video_save(VERA* vera, SDL_RWops* f) {
	SDL_RWwrite(f, &vera->video_ram[0], sizeof(uint8_t), sizeof(vera->video_ram));
	SDL_RWwrite(f, &vera->registers[0], sizeof(uint8_t), sizeof(vera->registers));
	SDL_RWwrite(f, &vera->palette[0], sizeof(uint8_t), sizeof(vera->palette));
	SDL_RWwrite(f, &vera->reg_layer[0][0], sizeof(uint8_t), sizeof(vera->reg_layer));
	SDL_RWwrite(f, &vera->sprite_data[0], sizeof(uint8_t), sizeof(vera->sprite_data));
}

uint8_t vera_read(VERA* vera, uint16_t reg) {
	bool ntsc_mode = vera->registers[0] & 2;
	uint16_t scanline = ntsc_mode ? vera->ntsc_scan_pos_y % SCAN_HEIGHT : vera->vga_scan_pos_y;
	if (scanline >= 512) scanline = 511;

	
	reg &= 0x03;

	switch (reg) {
	case 0x00:
		return scanline & 0xFF;
		break;
	case 0x01:
		return (scanline >> 8) & 0xFF;
		break;
	case 0x02:
	{
		if (vera->vram_reg_sel == 0) {
			uint32_t address = get_and_inc_address(vera, false);
			uint8_t value = vera->io_rddata;
			vera->io_rddata = video_space_read(vera, vera->io_addr);
			return value;
		}
		else {
			uint8_t value = vera->registers[vera->curr_reg];
			vera->curr_reg++;
			return value;
		}
		
	}
	break;
	//ISR
	case 0x03:
	{
		vera->vram_reg_sel = 0;
		return vera->isr;
	}
		
	break;
	}
	return 0;
}

void vera_write(VERA* vera, uint16_t reg, uint8_t value) {
	// if (reg > 4) {
	// 	printf("ioregisters[0x%02X] = 0x%02X\n", reg, value);
	// }
	//	printf("ioregisters[%d] = $%02X\n", reg, value);

	reg &= 0x3;
	switch (reg) {
	case 0x00:
		vera->io_addr = (vera->io_addr & 0xff00) | value;
		vera->io_rddata = video_space_read(vera, vera->io_addr);
		vera->vram_reg_sel = 0;
		break;
	case 0x01:
		vera->io_addr = (vera->io_addr & 0x00ff) | (value << 8);
		vera->io_rddata = video_space_read(vera, vera->io_addr);
		vera->vram_reg_sel = 0;
		break;
	case 0x02:
		if (vera->vram_reg_sel == 0) {
				if (enable_midline)
					vera_step(vera, MHZ, 0, true); // potential midline raster effect
				uint32_t address = get_and_inc_address(vera, true);
				if (log_video) {
					printf("WRITE video_space[$%X] = $%02X\n", address, value);
				}
				video_space_write(vera, address, value);
				vera->io_rddata = video_space_read(vera, vera->io_addr);
		}
		else {
			//write register
			process_register_write(vera,vera->curr_reg, value);
			vera->curr_reg++;
		}
		break;
	case 0x03:
		vera->curr_reg = value;
		vera->vram_reg_sel = 1;
		break;
	}
}

void process_register_write(VERA* vera, uint8_t reg, uint8_t value) {

	vera->registers[reg] = value;
	switch(reg) {
		//CTRL
		case 0:
			vera->video_palette.dirty = true;
			break;
		//IEN
		case 1:
			vera->irq_line = (vera->irq_line & 0xFF) | ((value >> 7) << 8);
			vera->ien = value & 0xF;
			break;
		//IRQ LINE [7:0]
		case 2:
			vera->irq_line = (vera->irq_line & 0x100) | value;
			break;
		//DC HSCALE
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			vera_step(vera, MHZ, 0, true); // potential midline raster effect
			vera->video_palette.dirty = true;
			break;
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF:
		case 0x10:
			vera_step(vera, MHZ, 0, true); // potential midline raster effect
			vera->reg_layer[0][reg - 0xA] = value;
			refresh_layer_properties(vera, 0);
			break;
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			vera_step(vera, MHZ, 0, true); // potential midline raster effect
			vera->reg_layer[1][reg - 0x11] = value;
			refresh_layer_properties(vera, 1);
			break;
		case REG_ADDR_INC:
			vera->io_inc = value;
			break;

	}
}

void video_update_title(VERA* vera, const char* window_title) {
	SDL_SetWindowTitle(vera->window, window_title);
}

uint8_t video_space_read(VERA* vera, uint32_t address) {
	return vera->video_ram[address & 0xFFFF];
}

void video_space_write(VERA* vera, uint32_t address, uint8_t value) {
	vera->video_ram[address & 0xFFFF] = value;

	if (address >= ADDR_PSG_START && address < ADDR_PSG_END) {
		//audio_render();
		//psg_writereg(address & 0x3f, value);
	}
	else if (address >= ADDR_PALETTE_START && address < ADDR_PALETTE_END) {
		vera->palette[address & 0x1ff] = value;
		vera->video_palette.dirty = true;
	}
	else if (address >= ADDR_SPRDATA_START && address < ADDR_SPRDATA_END) {
		vera->sprite_data[(address >> 3) & 0x7f][address & 0x7] = value;
		refresh_sprite_properties(vera, (address >> 3) & 0x7f);
	}
}

bool video_is_tilemap_address(VERA* vera, uint32_t addr) {
	for (int l = 0; l < 2; ++l) {
		struct video_layer_properties* props = &vera->layer_properties[l];
		if (addr < props->map_base) {
			continue;
		}
		if (addr >= props->map_base + (2 << (props->mapw_log2 + props->maph_log2))) {
			continue;
		}

		return true;
	}
	return false;
}

bool video_is_tiledata_address(VERA* vera, uint32_t addr) {
	for (int l = 0; l < 2; ++l) {
		struct video_layer_properties* props = &vera->layer_properties[l];
		if (addr < props->tile_base) {
			continue;
		}
		int tile_size = props->tilew * props->tileh * props->bits_per_pixel / 8;
		if (addr >= props->tile_base + tile_size * (props->bits_per_pixel == 1 ? 256 : 1024)) {
			continue;
		}

		return true;
	}
	return false;
}

bool video_is_special_address(uint32_t addr) {
	return addr >= 0x1F9C0;
}

uint32_t video_get_address(VERA* vera) {
	uint32_t address = vera->io_addr;
	return address & 0xffff;
}

static int calc_layer_eff_x(const struct video_layer_properties* props, const int x) {
	return (x + props->hscroll) & (props->layerw_max);
}

static int calc_layer_eff_y(const struct video_layer_properties* props, const int y) {
	return (y + props->vscroll) & (props->layerh_max);
}

static uint32_t calc_layer_map_addr_base2(const struct video_layer_properties* props, const int eff_x, const int eff_y) {
	// Slightly faster on some platforms because we know that tilew and tileh are powers of 2.
	return props->map_base + ((((eff_y >> props->tileh_log2) << props->mapw_log2) + (eff_x >> props->tilew_log2)) << 1);
}

static void refresh_layer_properties(VERA* vera, const uint8_t layer) {
	struct video_layer_properties* props = &vera->layer_properties[layer];

	uint16_t prev_layerw_max = props->layerw_max;
	uint16_t prev_hscroll = props->hscroll;

	props->color_depth = vera->reg_layer[layer][0] & 0x3;
	props->map_base = (vera->reg_layer[layer][1] & 0x7F) << 8;
	props->tile_base = (vera->reg_layer[layer][2] & 0xFC) << 9;
	props->bitmap_mode = (vera->reg_layer[layer][0] & 0x4) != 0;
	props->text_mode = (props->color_depth == 0) && !props->bitmap_mode;
	props->text_mode_256c = (vera->reg_layer[layer][0] & 8) != 0;
	props->tile_mode = !props->bitmap_mode && !props->text_mode;

	if (!props->bitmap_mode) {
		props->hscroll = vera->reg_layer[layer][3] | (vera->reg_layer[layer][4] & 0xf) << 8;
		props->vscroll = vera->reg_layer[layer][5] | (vera->reg_layer[layer][6] & 0xf) << 8;
	}
	else {
		props->hscroll = 0;
		props->vscroll = 0;
	}

	uint16_t mapw = 0;
	uint16_t maph = 0;
	props->tilew = 0;
	props->tileh = 0;

	if (props->tile_mode || props->text_mode) {
		props->mapw_log2 = 5 + ((vera->reg_layer[layer][0] >> 4) & 3);
		props->maph_log2 = 5 + ((vera->reg_layer[layer][0] >> 6) & 3);
		mapw = 1 << props->mapw_log2;
		maph = 1 << props->maph_log2;

		// Scale the tiles or text characters according to TILEW and TILEH.
		props->tilew_log2 = 3 + (vera->reg_layer[layer][2] & 1);
		props->tileh_log2 = 3 + ((vera->reg_layer[layer][2] >> 1) & 1);
		props->tilew = 1 << props->tilew_log2;
		props->tileh = 1 << props->tileh_log2;
	}
	else if (props->bitmap_mode) {
		// bitmap mode is basically tiled mode with a single huge tile
		props->tilew = (vera->reg_layer[layer][2] & 1) ? 640 : 320;
		props->tileh = SCREEN_HEIGHT;
	}

	// We know mapw, maph, tilew, and tileh are powers of two in all cases except bitmap modes, and any products of that set will be powers of two,
	// so there's no need to modulo against them if we have bitmasks we can bitwise-and against.

	props->mapw_max = mapw - 1;
	props->maph_max = maph - 1;
	props->tilew_max = props->tilew - 1;
	props->tileh_max = props->tileh - 1;
	props->layerw_max = (mapw * props->tilew) - 1;
	props->layerh_max = (maph * props->tileh) - 1;

	// Find min/max eff_x for bulk reading in tile data during draw.
	if (prev_layerw_max != props->layerw_max || prev_hscroll != props->hscroll) {
		int min_eff_x = INT_MAX;
		int max_eff_x = INT_MIN;
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			int eff_x = calc_layer_eff_x(props, x);
			if (eff_x < min_eff_x) {
				min_eff_x = eff_x;
			}
			if (eff_x > max_eff_x) {
				max_eff_x = eff_x;
			}
		}
		props->min_eff_x = min_eff_x;
		props->max_eff_x = max_eff_x;
	}

	props->bits_per_pixel = 1 << props->color_depth;
	props->tile_size_log2 = props->tilew_log2 + props->tileh_log2 + props->color_depth - 3;

	props->first_color_pos = 8 - props->bits_per_pixel;
	props->color_mask = (1 << props->bits_per_pixel) - 1;
	props->color_fields_max = (8 >> props->color_depth) - 1;
}

static void screenshot(void) {
	//char path[PATH_MAX]; 
	//const time_t now = time(NULL);
	//strftime(path, PATH_MAX, "x16emu-%Y-%m-%d-%H-%M-%S.png", localtime(&now));

	//memset(png_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 3);

	//// The framebuffer stores pixels in BRGA but we want RGB:
	//for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
	//	png_buffer[(i * 3) + 0] = framebuffer[(i * 4) + 2];
	//	png_buffer[(i * 3) + 1] = framebuffer[(i * 4) + 1];
	//	png_buffer[(i * 3) + 2] = framebuffer[(i * 4) + 0];
	//}

	//if (stbi_write_png(path, SCREEN_WIDTH, SCREEN_HEIGHT, 3, png_buffer, SCREEN_WIDTH * 3)) {
	//	printf("Wrote screenshot to %s\n", path);
	//}
	//else {
	//	printf("WARNING: Couldn't write screenshot to %s\n", path);
	//}
}

static void refresh_sprite_properties(VERA* vera, const uint16_t sprite) {

	struct video_sprite_properties* props = &vera->sprite_properties[sprite];

	props->sprite_zdepth = (vera->sprite_data[sprite][6] >> 2) & 3;
	props->sprite_collision_mask = vera->sprite_data[sprite][6] & 0xf0;

	props->sprite_x = vera->sprite_data[sprite][2] | (vera->sprite_data[sprite][3] & 3) << 8;
	props->sprite_y = vera->sprite_data[sprite][4] | (vera->sprite_data[sprite][5] & 3) << 8;
	props->sprite_width_log2 = (((vera->sprite_data[sprite][7] >> 4) & 3) + 3);
	props->sprite_height_log2 = ((vera->sprite_data[sprite][7] >> 6) + 3);
	props->sprite_width = 1 << props->sprite_width_log2;
	props->sprite_height = 1 << props->sprite_height_log2;

	// fix up negative coordinates
	if (props->sprite_x >= 0x400 - props->sprite_width) {
		props->sprite_x -= 0x400;
	}
	if (props->sprite_y >= 0x400 - props->sprite_height) {
		props->sprite_y -= 0x400;
	}

	props->hflip = vera->sprite_data[sprite][6] & 1;
	props->vflip = (vera->sprite_data[sprite][6] >> 1) & 1;

	props->color_mode = (vera->sprite_data[sprite][1] >> 7) & 1;
	props->sprite_address = vera->sprite_data[sprite][0] << 5 | (vera->sprite_data[sprite][1] & 0xf) << 13;

	props->palette_offset = (vera->sprite_data[sprite][7] & 0x0f) << 4;
}

static void refresh_palette(VERA* vera) {
	const uint8_t out_mode = vera->registers[0] & 3;
	const bool chroma_disable = ((vera->registers[0] & 0x07) == 6);
	for (int i = 0; i < 256; ++i) {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		if (out_mode == 0) {
			// video generation off
			// -> show blue screen
			r = 0;
			g = 0;
			b = 255;
		}
		else {
			uint16_t entry = vera->palette[i * 2] | vera->palette[i * 2 + 1] << 8;
			r = ((entry >> 8) & 0xf) << 4 | ((entry >> 8) & 0xf);
			g = ((entry >> 4) & 0xf) << 4 | ((entry >> 4) & 0xf);
			b = (entry & 0xf) << 4 | (entry & 0xf);
			if (chroma_disable) {
				r = g = b = (r + b + g) / 3;
			}
		}

		vera->video_palette.entries[i] = (uint32_t)(r << 16) | ((uint32_t)g << 8) | ((uint32_t)b);
	}
	vera->video_palette.dirty = false;
}

static void expand_4bpp_data(uint8_t* dst, const uint8_t* src, int dst_size) {
	while (dst_size >= 2) {
		*dst = (*src) >> 4;
		++dst;
		*dst = (*src) & 0xf;
		++dst;

		++src;
		dst_size -= 2;
	}
}

static void render_sprite_line(VERA* vera, const uint16_t y) {
	memset(vera->sprite_line_col, 0, SCREEN_WIDTH);
	memset(vera->sprite_line_z, 0, SCREEN_WIDTH);
	memset(vera->sprite_line_mask, 0, SCREEN_WIDTH);

	uint16_t sprite_budget = 800 + 1;
	for (int i = 0; i < NUM_SPRITES; i++) {
		// one clock per lookup
		sprite_budget--; if (sprite_budget == 0) break;
		const struct video_sprite_properties* props = &vera->sprite_properties[i];

		if (props->sprite_zdepth == 0) {
			continue;
		}

		// check whether this line falls within the sprite
		if (y < props->sprite_y || y >= props->sprite_y + props->sprite_height) {
			continue;
		}

		const uint16_t eff_sy = props->vflip ? ((props->sprite_height - 1) - (y - props->sprite_y)) : (y - props->sprite_y);

		int16_t       eff_sx = (props->hflip ? (props->sprite_width - 1) : 0);
		const int16_t eff_sx_incr = props->hflip ? -1 : 1;

		const uint8_t* bitmap_data = vera->video_ram + props->sprite_address + (eff_sy << (props->sprite_width_log2 - (1 - props->color_mode)));

		uint8_t unpacked_sprite_line[64];
		const uint16_t width = (props->sprite_width < 64 ? props->sprite_width : 64);
		if (props->color_mode == 0) {
			// 4bpp
			expand_4bpp_data(unpacked_sprite_line, bitmap_data, width);
		}
		else {
			// 8bpp
			memcpy(unpacked_sprite_line, bitmap_data, width);
		}

		for (uint16_t sx = 0; sx < props->sprite_width; ++sx) {
			const uint16_t line_x = props->sprite_x + sx;
			if (line_x >= SCREEN_WIDTH) {
				eff_sx += eff_sx_incr;
				continue;
			}

			// one clock per fetched 32 bits
			if (!(sx & 3)) {
				sprite_budget--; if (sprite_budget == 0) break;
			}

			// one clock per rendered pixel
			sprite_budget--; if (sprite_budget == 0) break;

			const uint8_t col_index = unpacked_sprite_line[eff_sx];
			eff_sx += eff_sx_incr;

			// palette offset
			if (col_index > 0) {
				vera->sprite_line_collisions |= vera->sprite_line_mask[line_x] & props->sprite_collision_mask;
				vera->sprite_line_mask[line_x] |= props->sprite_collision_mask;

				if (props->sprite_zdepth > vera->sprite_line_z[line_x]) {
					vera->sprite_line_col[line_x] = col_index + props->palette_offset;
					vera->sprite_line_z[line_x] = props->sprite_zdepth;
				}
			}
		}
	}
}

static void render_layer_line_text(VERA* vera, uint8_t layer, uint16_t y) {
	const struct video_layer_properties* props = &vera->prev_layer_properties[1][layer];
	const struct video_layer_properties* props0 = &vera->prev_layer_properties[0][layer];

	const uint8_t max_pixels_per_byte = (8 >> props->color_depth) - 1;
	const int     eff_y = calc_layer_eff_y(props0, y);
	const int     yy = eff_y & props->tileh_max;

	// additional bytes to reach the correct line of the tile
	const uint32_t y_add = (yy << props->tilew_log2) >> 3;

	const uint32_t map_addr_begin = calc_layer_map_addr_base2(props, props->min_eff_x, eff_y);
	const uint32_t map_addr_end = calc_layer_map_addr_base2(props, props->max_eff_x, eff_y);
	const int      size = (map_addr_end - map_addr_begin) + 2;

	uint8_t tile_bytes[512]; // max 256 tiles, 2 bytes each.
	video_space_read_range(vera, tile_bytes, map_addr_begin, size);

	uint32_t tile_start;

	uint8_t  fg_color;
	uint8_t  bg_color;
	uint8_t  s;
	uint8_t  color_shift;

	{
		const int eff_x = calc_layer_eff_x(props, 0);
		const int xx = eff_x & props->tilew_max;

		// extract all information from the map
		const uint32_t map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

		const uint8_t tile_index = tile_bytes[map_addr];
		const uint8_t byte1 = tile_bytes[map_addr + 1];

		if (!props->text_mode_256c) {
			fg_color = byte1 & 15;
			bg_color = byte1 >> 4;
		}
		else {
			fg_color = byte1;
			bg_color = 0;
		}

		// offset within tilemap of the current tile
		tile_start = tile_index << props->tile_size_log2;

		// additional bytes to reach the correct column of the tile
		const uint16_t x_add = xx >> 3;
		const uint32_t tile_offset = tile_start + y_add + x_add;

		s = video_space_read(vera, props->tile_base + tile_offset);
		color_shift = max_pixels_per_byte - (xx & 0x7);
	}

	// Render tile line.
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		// Scrolling
		const int eff_x = calc_layer_eff_x(props, x);
		const int xx = eff_x & props->tilew_max;

		if ((eff_x & 0x7) == 0) {
			if ((eff_x & props->tilew_max) == 0) {
				// extract all information from the map
				const uint32_t map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

				const uint8_t tile_index = tile_bytes[map_addr];
				const uint8_t byte1 = tile_bytes[map_addr + 1];

				if (!props->text_mode_256c) {
					fg_color = byte1 & 15;
					bg_color = byte1 >> 4;
				}
				else {
					fg_color = byte1;
					bg_color = 0;
				}

				// offset within tilemap of the current tile
				tile_start = tile_index << props->tile_size_log2;
			}

			// additional bytes to reach the correct column of the tile
			const uint16_t x_add = xx >> 3;
			const uint32_t tile_offset = tile_start + y_add + x_add;

			s = video_space_read(vera, props->tile_base + tile_offset);
			color_shift = max_pixels_per_byte;
		}

		// convert tile byte to indexed color
		const uint8_t col_index = (s >> color_shift) & 1;
		--color_shift;
		vera->layer_line[layer][x] = col_index ? fg_color : bg_color;
	}
}

static void render_layer_line_tile(VERA* vera, uint8_t layer, uint16_t y) {
	const struct video_layer_properties* props = &vera->prev_layer_properties[1][layer];
	const struct video_layer_properties* props0 = &vera->prev_layer_properties[0][layer];

	const uint8_t max_pixels_per_byte = (8 >> props->color_depth) - 1;
	const int     eff_y = calc_layer_eff_y(props0, y);
	const uint8_t yy = eff_y & props->tileh_max;
	const uint8_t yy_flip = yy ^ props->tileh_max;
	const uint32_t y_add = (yy << ((props->tilew_log2 + props->color_depth - 3) & 31));
	const uint32_t y_add_flip = (yy_flip << ((props->tilew_log2 + props->color_depth - 3) & 31));

	const uint32_t map_addr_begin = calc_layer_map_addr_base2(props, props->min_eff_x, eff_y);
	const uint32_t map_addr_end = calc_layer_map_addr_base2(props, props->max_eff_x, eff_y);
	const int      size = (map_addr_end - map_addr_begin) + 2;

	uint8_t tile_bytes[512]; // max 256 tiles, 2 bytes each.
	video_space_read_range(vera, tile_bytes, map_addr_begin, size);

	uint8_t  palette_offset;
	bool     vflip;
	bool     hflip;
	uint32_t tile_start;
	uint8_t  s;
	uint8_t  color_shift;
	int8_t   color_shift_incr;

	{
		const int eff_x = calc_layer_eff_x(props, 0);

		// extract all information from the map
		const uint32_t map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

		const uint8_t byte0 = tile_bytes[map_addr];
		const uint8_t byte1 = tile_bytes[map_addr + 1];

		// Tile Flipping
		vflip = (byte1 >> 3) & 1;
		hflip = (byte1 >> 2) & 1;

		palette_offset = byte1 & 0xf0;

		// offset within tilemap of the current tile
		const uint16_t tile_index = byte0 | ((byte1 & 3) << 8);
		tile_start = tile_index << props->tile_size_log2;

		color_shift_incr = hflip ? props->bits_per_pixel : -props->bits_per_pixel;

		int xx = eff_x & props->tilew_max;
		if (hflip) {
			xx = xx ^ (props->tilew_max);
			color_shift = 0;
		}
		else {
			color_shift = props->first_color_pos;
		}

		// additional bytes to reach the correct column of the tile
		uint16_t x_add = (xx << props->color_depth) >> 3;
		uint32_t tile_offset = tile_start + (vflip ? y_add_flip : y_add) + x_add;

		s = video_space_read(vera, props->tile_base + tile_offset);
	}


	// Render tile line.
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		const int eff_x = calc_layer_eff_x(props, x);

		if ((eff_x & max_pixels_per_byte) == 0) {
			if ((eff_x & props->tilew_max) == 0) {
				// extract all information from the map
				const uint32_t map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

				const uint8_t byte0 = tile_bytes[map_addr];
				const uint8_t byte1 = tile_bytes[map_addr + 1];

				// Tile Flipping
				vflip = (byte1 >> 3) & 1;
				hflip = (byte1 >> 2) & 1;

				palette_offset = byte1 & 0xf0;

				// offset within tilemap of the current tile
				const uint16_t tile_index = byte0 | ((byte1 & 3) << 8);
				tile_start = tile_index << props->tile_size_log2;

				color_shift_incr = hflip ? props->bits_per_pixel : -props->bits_per_pixel;
			}

			int xx = eff_x & props->tilew_max;
			if (hflip) {
				xx = xx ^ (props->tilew_max);
				color_shift = 0;
			}
			else {
				color_shift = props->first_color_pos;
			}

			// additional bytes to reach the correct column of the tile
			const uint16_t x_add = (xx << props->color_depth) >> 3;
			const uint32_t tile_offset = tile_start + (vflip ? y_add_flip : y_add) + x_add;

			s = video_space_read(vera, props->tile_base + tile_offset);
		}

		// convert tile byte to indexed color
		uint8_t col_index = (s >> color_shift) & props->color_mask;
		color_shift += color_shift_incr;

		// Apply Palette Offset
		if (col_index > 0 && col_index < 16) {
			col_index += palette_offset;
			if (props->text_mode_256c) {
				col_index |= 0x80;
			}
		}
		vera->layer_line[layer][x] = col_index;
	}
}

static void render_layer_line_bitmap(VERA* vera, uint8_t layer, uint16_t y) {
	const struct video_layer_properties* props = &vera->prev_layer_properties[1][layer];
	//	const struct video_layer_properties *props0 = &prev_layer_properties[0][layer];

	int yy = y % props->tileh;
	// additional bytes to reach the correct line of the tile
	uint32_t y_add = (yy * props->tilew * props->bits_per_pixel) >> 3;

	// Render tile line.
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		int xx = x % props->tilew;

		// extract all information from the map
		uint8_t palette_offset = vera->reg_layer[layer][4] & 0xf;

		// additional bytes to reach the correct column of the tile
		uint16_t x_add = (xx * props->bits_per_pixel) >> 3;
		uint32_t tile_offset = y_add + x_add;
		uint8_t s = video_space_read(vera, props->tile_base + tile_offset);

		// convert tile byte to indexed color
		uint8_t col_index = (s >> (props->first_color_pos - ((xx & props->color_fields_max) << props->color_depth))) & props->color_mask;

		// Apply Palette Offset
		if (col_index > 0 && col_index < 16) {
			col_index += palette_offset << 4;
			if (props->text_mode_256c) {
				col_index |= 0x80;
			}
		}
		vera->layer_line[layer][x] = col_index;
	}
}

static uint8_t calculate_line_col_index(uint8_t spr_zindex, uint8_t spr_col_index, uint8_t l1_col_index, uint8_t l2_col_index) {
	uint8_t col_index = 0;
	switch (spr_zindex) {
	case 3:
		col_index = spr_col_index ? spr_col_index : (l2_col_index ? l2_col_index : l1_col_index);
		break;
	case 2:
		col_index = l2_col_index ? l2_col_index : (spr_col_index ? spr_col_index : l1_col_index);
		break;
	case 1:
		col_index = l2_col_index ? l2_col_index : (l1_col_index ? l1_col_index : spr_col_index);
		break;
	case 0:
		col_index = l2_col_index ? l2_col_index : l1_col_index;
		break;
	}
	return col_index;
}

static void render_line(VERA* vera, uint16_t y, float scan_pos_x) {
	static uint16_t y_prev;
	static uint16_t s_pos_x_p;
	static uint32_t eff_y_fp; // 16.16 fixed point
	static uint32_t eff_x_fp; // 16.16 fixed point

	static uint8_t col_line[SCREEN_WIDTH];

	uint8_t dc_video = vera->registers[REG_CTRL];
	uint16_t vstart = vera->registers[REG_VSTART] << 1;

	if (y != y_prev) {
		y_prev = y;
		s_pos_x_p = 0;

		// Copy the composer array to 2-line history buffer
		// so that the raster effects that happen on a delay take effect
		// at exactly the right time

		// This simulates different effects happening at render,
		// render but delayed until the next line, or applied mid-line
		// at scan-out

		memcpy(vera->prev_registers[1], vera->prev_registers[0], sizeof(*vera->registers) * 256);
		memcpy(vera->prev_registers[0], vera->registers, sizeof(*vera->registers) * 256);

		// Same with the layer properties

		memcpy(vera->prev_layer_properties[1], vera->prev_layer_properties[0], sizeof(*vera->layer_properties) * NUM_LAYERS);
		memcpy(vera->prev_layer_properties[0], vera->layer_properties, sizeof(*vera->layer_properties) * NUM_LAYERS);

		if ((dc_video & 3) > 1) { // 480i or 240p
			if ((y >> 1) == 0) {
				eff_y_fp = y * (vera->prev_registers[1][0x4] << 9);
			}
			else if ((y & 0xfffe) > vstart) {
				eff_y_fp += (vera->prev_registers[1][0x4] << 10);
			}
		}
		else {
			if (y == 0) {
				eff_y_fp = 0;
			}
			else if (y > vstart) {
				eff_y_fp += (vera->prev_registers[1][0x4] << 9);
			}
		}
	}

	if ((dc_video & 8) && (dc_video & 3) > 1) { // progressive NTSC/RGB mode
		y &= 0xfffe;
	}

	// refresh palette for next entry
	if (vera->video_palette.dirty) {
		refresh_palette(vera);
	}

	if (y >= SCREEN_HEIGHT) {
		return;
	}

	uint16_t s_pos_x = (uint16_t)round(scan_pos_x);
	if (s_pos_x > SCREEN_WIDTH) {
		s_pos_x = SCREEN_WIDTH;
	}

	if (s_pos_x_p == 0) {
		eff_x_fp = 0;
	}

	uint8_t out_mode = vera->registers[0] & 3;

	uint8_t border_color = vera->registers[REG_BORDER_COLOR];
	uint16_t hstart = vera->registers[REG_HSTART] << 2;
	uint16_t hstop = vera->registers[REG_HSTOP] << 2;
	uint16_t vstop = vera->registers[REG_VSTOP] << 1;

	uint16_t eff_y = (eff_y_fp >> 16);
	if (eff_y >= 480) eff_y = 480 - (y & 1);

	vera->layer_line_enable[0] = dc_video & 0x10;
	vera->layer_line_enable[1] = dc_video & 0x20;
	vera->sprite_line_enable = dc_video & 0x40;

	// clear layer_line if layer gets disabled
	for (uint8_t layer = 0; layer < 2; layer++) {
		if (!vera->layer_line_enable[layer] && vera->old_layer_line_enable[layer]) {
			for (uint16_t i = s_pos_x_p; i < SCREEN_WIDTH; i++) {
				vera->layer_line[layer][i] = 0;
			}
		}
		if (s_pos_x_p == 0)
			vera->old_layer_line_enable[layer] = vera->layer_line_enable[layer];
	}

	// clear sprite_line if sprites get disabled
	if (!vera->sprite_line_enable && vera->old_sprite_line_enable) {
		for (uint16_t i = s_pos_x_p; i < SCREEN_WIDTH; i++) {
			vera->sprite_line_col[i] = 0;
			vera->sprite_line_z[i] = 0;
			vera->sprite_line_mask[i] = 0;
		}
	}

	if (s_pos_x_p == 0)
		vera->old_sprite_line_enable = vera->sprite_line_enable;



	if (vera->sprite_line_enable) {
		render_sprite_line(vera, eff_y);
	}

	if (warp_mode && (vera->frame_count & 63)) {
		// sprites were needed for the collision IRQ, but we can skip
		// everything else if we're in warp mode, most of the time
		return;
	}

	if (vera->layer_line_enable[0]) {
		if (vera->prev_layer_properties[1][0].text_mode) {
			render_layer_line_text(vera, 0, eff_y);
		}
		else if (vera->prev_layer_properties[1][0].bitmap_mode) {
			render_layer_line_bitmap(vera, 0, eff_y);
		}
		else {
			render_layer_line_tile(vera, 0, eff_y);
		}
	}
	if (vera->layer_line_enable[1]) {
		if (vera->prev_layer_properties[1][1].text_mode) {
			render_layer_line_text(vera, 1, eff_y);
		}
		else if (vera->prev_layer_properties[1][1].bitmap_mode) {
			render_layer_line_bitmap(vera, 1, eff_y);
		}
		else {
			render_layer_line_tile(vera, 1, eff_y);
		}
	}

	// If video output is enabled, calculate color indices for line.
	if (out_mode != 0) {
		// Add border after if required.
		if (y < vstart || y >= vstop) {
			uint32_t border_fill = border_color;
			border_fill = border_fill | (border_fill << 8);
			border_fill = border_fill | (border_fill << 16);
			memset(col_line, border_fill, SCREEN_WIDTH);
		}
		else {
			hstart = hstart < 640 ? hstart : 640;
			hstop = hstop < 640 ? hstop : 640;

			for (uint16_t x = s_pos_x_p; x < hstart && x < s_pos_x; ++x) {
				col_line[x] = border_color;
			}

			const uint32_t scale = vera->registers[0x3];
			for (uint16_t x = MAX(hstart, s_pos_x_p); x < hstop && x < s_pos_x; ++x) {
				uint16_t eff_x = eff_x_fp >> 16;
				col_line[x] = (eff_x < SCREEN_WIDTH) ? calculate_line_col_index(vera->sprite_line_z[eff_x], vera->sprite_line_col[eff_x], vera->layer_line[0][eff_x], vera->layer_line[1][eff_x]) : 0;
				eff_x_fp += (scale << 9);
			}
			for (uint16_t x = hstop; x < s_pos_x; ++x) {
				col_line[x] = border_color;
			}
		}
	}

	// Look up all color indices.
	uint32_t* framebuffer4_begin = ((uint32_t*)vera->framebuffer) + (y * SCREEN_WIDTH) + s_pos_x_p;
	{
		uint32_t* framebuffer4 = framebuffer4_begin;
		for (uint16_t x = s_pos_x_p; x < s_pos_x; x++) {
			*framebuffer4++ = vera->video_palette.entries[col_line[x]];
		}
	}

	// NTSC overscan
	if (out_mode == 2) {
		uint32_t* framebuffer4 = framebuffer4_begin;
		for (uint16_t x = s_pos_x_p; x < s_pos_x; x++) {
			if (x < SCREEN_WIDTH * TITLE_SAFE_X ||
				x > SCREEN_WIDTH * (1 - TITLE_SAFE_X) ||
				y < SCREEN_HEIGHT * TITLE_SAFE_Y ||
				y > SCREEN_HEIGHT * (1 - TITLE_SAFE_Y)) {

				// Divide RGB elements by 4.
				*framebuffer4 &= 0x00fcfcfc;
				*framebuffer4 >>= 2;
			}
			framebuffer4++;
		}
	}

	s_pos_x_p = s_pos_x;
}

static void update_isr_and_coll(VERA* vera, uint16_t y, uint16_t compare) {
	if (y == SCREEN_HEIGHT) {
		if (vera->sprite_line_collisions != 0) {
			vera->isr |= 4;
		}
		vera->isr = (vera->isr & 0xf) | vera->sprite_line_collisions;
		vera->sprite_line_collisions = 0;
		vera->isr |= 1; // VSYNC IRQ

		i8259_do_irq(&machine.i8259, 2);
		//doirq(2);

	}
	if (y == compare) { // LINE IRQ
		vera->isr |= 2;
	}
}


//
// saves the video memory and register content into a file
//




uint32_t get_and_inc_address(VERA* vera, bool write) {
	uint32_t address = vera->io_addr;
	int16_t incr = increments[vera->io_inc];
	vera->io_addr += incr;

	return address;
}

//
// Vera: Internal Video Address Space
//



static void video_space_read_range(VERA* vera, uint8_t* dest, uint32_t address, uint32_t size) {
	if (address >= ADDR_VRAM_START && (address + size) <= ADDR_VRAM_END) {
		memcpy(dest, &vera->video_ram[address], size);
	}
	else {
		for (uint32_t i = 0; i < size; ++i) {
			*dest++ = video_space_read(vera, address + i);
		}
	}
}

//void print_palette_fgpa() {
//	for (int i = 0; i < 256; i++) {
//		printf("ram[%d] = 16'h%04X;\n", i, default_palette[i]);
//	}
//}

//void export_palette_to_24bit() {
//
//	unsigned char c;
//	FILE* fp;
//
//	/* open a file to write */
//	fp = fopen("pal.dat", "w");
//	if (fp == NULL) {
//		printf("Open Error\n");
//		return;
//	}
//
//	for (int i = 0; i < 256; i++) {
//		uint16_t entry = palette[i * 2] | palette[i * 2 + 1] << 8;
//		uint8_t r = ((entry >> 8) & 0xf) << 4 | ((entry >> 8) & 0xf);
//		uint8_t g = ((entry >> 4) & 0xf) << 4 | ((entry >> 4) & 0xf);
//		uint8_t b = (entry & 0xf) << 4 | (entry & 0xf);
//		fputc(r, fp);
//		fputc(g, fp);
//		fputc(b, fp);
//	}
//
//	/* close file */
//	fclose(fp);
//}


void vera_load_default_font(uint32_t addr) {

	//clear out first 64K vram
	vera_write(&machine.vera, 0x03, REG_ADDR_INC);
	vera_write(&machine.vera, 0x02, 2);
	vera_write(&machine.vera, 0x00, 0x00);
	vera_write(&machine.vera, 0x01, 0x00);
	for (int i = 0; i <= 0xFFFF; i++) {
		vera_write(&machine.vera, 0x2, 0);
	}

	//load font
	vera_write(&machine.vera, 0, addr & 0xFF);
	vera_write(&machine.vera, 1, (addr >> 8) & 0xFF);
	for (int c = 0; c < 128; c++) {
		for (int i = 0; i < 8; i++) {
			vera_write(&machine.vera, 0x2, default_font[c * 8 + i]);
		}
	}

	vera_write(&machine.vera, 0x03, REG_CTRL);
	vera_write(&machine.vera, 0x02, 0b00010001); //ctrl L0 enabled, display on
	vera_write(&machine.vera, 0x02, 0); //ien
	vera_write(&machine.vera, 0x02, 0); //irq line l
	vera_write(&machine.vera, 0x02, 128); //hscale
	vera_write(&machine.vera, 0x02, 128); //vscale
	vera_write(&machine.vera, 0x02, 0); //border


	vera_write(&machine.vera, 0x03, REG_L0_CONFIG);
	vera_write(&machine.vera, 0x02, 0b01100000); // L0 config
	vera_write(&machine.vera, 0x02, 0b00000000); // Map base 0x0000
	vera_write(&machine.vera, 0x02, 0b01111000); // Tile base 0xF000
	vera_write(&machine.vera, 0x02, 0);
	vera_write(&machine.vera, 0x02, 0);
	vera_write(&machine.vera, 0x02, 0);
	vera_write(&machine.vera, 0x02, 0);


	vera_write(&machine.vera, 0x00, 0x00);
	vera_write(&machine.vera, 0x01, 0x02);
	vera_write(&machine.vera, 0x2, 'H');
	vera_write(&machine.vera, 0x2, 0x01);
	vera_write(&machine.vera, 0x2, 'E');
	vera_write(&machine.vera, 0x2, 0x01);
	vera_write(&machine.vera, 0x2, 'L');
	vera_write(&machine.vera, 0x2, 0x01);
	vera_write(&machine.vera, 0x2, 'L');
	vera_write(&machine.vera, 0x2, 0x01);
	vera_write(&machine.vera, 0x2, '0');
	vera_write(&machine.vera, 0x2, 0x01);
	vera_write(&machine.vera, 0x2, '!');
	vera_write(&machine.vera, 0x2, 0x07);

}





