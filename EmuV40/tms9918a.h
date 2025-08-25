#ifndef TMS9918A_H
#define TMS9918A_H

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum
{
	TMS_MODE_GRAPHICS_I,
	TMS_MODE_GRAPHICS_II,
	TMS_MODE_TEXT,
	TMS_MODE_MULTICOLOR,
} TMS9918A_MODE;

typedef enum
{
	TMS_TRANSPARENT = 0,
	TMS_BLACK,
	TMS_MED_GREEN,
	TMS_LT_GREEN,
	TMS_DK_BLUE,
	TMS_LT_BLUE,
	TMS_DK_RED,
	TMS_CYAN,
	TMS_MED_RED,
	TMS_LT_RED,
	TMS_DK_YELLOW,
	TMS_LT_YELLOW,
	TMS_DK_GREEN,
	TMS_MAGENTA,
	TMS_GREY,
	TMS_WHITE,
} TMS9918A_COLOR;

typedef enum
{
	TMS_REG_0 = 0,
	TMS_REG_1,
	TMS_REG_2,
	TMS_REG_3,
	TMS_REG_4,
	TMS_REG_5,
	TMS_REG_6,
	TMS_REG_7,
	TMS_NUM_REGISTERS,
	TMS_REG_NAME_TABLE = TMS_REG_2,
	TMS_REG_COLOR_TABLE = TMS_REG_3,
	TMS_REG_PATTERN_TABLE = TMS_REG_4,
	TMS_REG_SPRITE_ATTR_TABLE = TMS_REG_5,
	TMS_REG_SPRITE_PATT_TABLE = TMS_REG_6,
	TMS_REG_FG_BG_COLOR = TMS_REG_7,
} TMS9918A_REGISTER;

#define TMS9918_PIXELS_X			256
#define TMS9918_PIXELS_Y			192

#define VRAM_SIZE					(1 << 14) /* 16KB */
#define VRAM_MASK					(VRAM_SIZE - 1) /* 0x3fff */

#define GRAPHICS_NUM_COLS			32
#define GRAPHICS_NUM_ROWS			24
#define GRAPHICS_CHAR_WIDTH			8

#define TEXT_NUM_COLS				40
#define TEXT_NUM_ROWS				24
#define TEXT_CHAR_WIDTH				6
#define TEXT_PADDING_PX				8

#define PATTERN_BYTES				8
#define GFXI_COLOR_GROUP_SIZE		8

#define MAX_SPRITES					32

#define SPRITE_ATTR_Y				0
#define SPRITE_ATTR_X				1
#define SPRITE_ATTR_NAME			2
#define SPRITE_ATTR_COLOR			3
#define SPRITE_ATTR_BYTES			4
#define LAST_SPRITE_YPOS			0xD0
#define MAX_SCANLINE_SPRITES		4

#define STATUS_INT					0x80
#define STATUS_5S					0x40
#define STATUS_COL					0x20

#define TMS_R0_MODE_GRAPHICS_I		0x00
#define TMS_R0_MODE_GRAPHICS_II		0x02
#define TMS_R0_MODE_MULTICOLOR		0x00
#define TMS_R0_MODE_TEXT			0x00
#define TMS_R0_EXT_VDP_ENABLE		0x01
#define TMS_R0_EXT_VDP_DISABLE		0x00

#define TMS_R1_RAM_16K				0x80
#define TMS_R1_RAM_4K				0x00
#define TMS_R1_DISP_BLANK			0x00
#define TMS_R1_DISP_ACTIVE			0x40
#define TMS_R1_INT_ENABLE			0x20
#define TMS_R1_INT_DISABLE			0x00
#define TMS_R1_MODE_GRAPHICS_I		0x00
#define TMS_R1_MODE_GRAPHICS_II		0x00
#define TMS_R1_MODE_MULTICOLOR		0x08
#define TMS_R1_MODE_TEXT			0x10
#define TMS_R1_SPRITE_8				0x00
#define TMS_R1_SPRITE_16			0x02
#define TMS_R1_SPRITE_MAG1			0x00
#define TMS_R1_SPRITE_MAG2			0x01

#define TMS_DEFAULT_VRAM_NAME_ADDRESS          0x3800
#define TMS_DEFAULT_VRAM_COLOR_ADDRESS         0x0000
#define TMS_DEFAULT_VRAM_PATT_ADDRESS          0x2000
#define TMS_DEFAULT_VRAM_SPRITE_ATTR_ADDRESS   0x3B00
#define TMS_DEFAULT_VRAM_SPRITE_PATT_ADDRESS   0x1800

#define LAST_SPRITE_YPOS        0xD0

/* tms9918 constants */
#define TMS9918_DISPLAY_WIDTH   320
#define TMS9918_DISPLAY_HEIGHT  240
#define TMS9918_FPS             60.0f
#define TMS9918_TICK_MIN_PIXELS 26

/* tms9918 computed constants */
#define TMS9918_FRAME_TIME      (1.0f / TMS9918_FPS)
#define TMS9918_ROW_TIME        (TMS9918_FRAME_TIME / (float)TMS9918_DISPLAY_HEIGHT)
#define TMS9918_PIXEL_TIME      (TMS9918_ROW_TIME / (float)TMS9918_DISPLAY_WIDTH)
#define TMS9918_BORDER_X        ((TMS9918_DISPLAY_WIDTH - TMS9918_PIXELS_X) / 2)
#define TMS9918_BORDER_Y        ((TMS9918_DISPLAY_HEIGHT - TMS9918_PIXELS_Y) / 2)
#define TMS9918_DISPLAY_PIXELS  (TMS9918_DISPLAY_WIDTH * TMS9918_DISPLAY_HEIGHT)

struct TMS9918_s
{
	/* the eight write-only registers */
	uint8_t registers[TMS_NUM_REGISTERS];

	/* status register (read-only) */
	uint8_t status;

	/* current address for cpu access (auto-increments) */
	uint16_t currentAddress;

	/* address or register write stage (0 or 1) */
	uint8_t regWriteStage;

	/* holds first stage of write to address/register port */
	uint8_t regWriteStage0Value;

	/* buffered value */
	uint8_t readAheadBuffer;

	/* current display mode */
	TMS9918A_MODE mode;

	/* video ram */
	uint8_t vram[VRAM_SIZE];

	uint8_t rowSpriteBits[TMS9918_PIXELS_X]; /* collision mask */
};

typedef struct TMS9918_s TMS9918A;


TMS9918A* tmsNew();
void tmsReset(TMS9918A* vdp);
void tmsDestroy(TMS9918A* vdp);
void tmsWriteAddr(TMS9918A* vdp, uint8_t data);
void tmsWriteData(TMS9918A* vdp, uint8_t data);
uint8_t tmsReadStatus(TMS9918A* vdp);
uint8_t tmsReadData(TMS9918A* vdp);
uint8_t tmsReadDataNoInc(TMS9918A* vdp);
void tmsScanline(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X]);
uint8_t tmsRegValue(TMS9918A* vdp, TMS9918A_REGISTER reg);
void tmsWriteRegValue(TMS9918A* vdp, TMS9918A_REGISTER reg, uint8_t value);
uint8_t tmsVramValue(TMS9918A* vdp, uint16_t addr);
bool tmsDisplayEnabled(TMS9918A* vdp);
TMS9918A_MODE tmsDisplayMode(TMS9918A* vdp);

// UTILS
extern uint32_t tmsPalette[];
/*
 * Write a register value
 */
inline static void tmsWriteRegisterValue(TMS9918A* vdp, TMS9918A_REGISTER reg, uint8_t value)
{
	tmsWriteAddr(vdp, value);
	tmsWriteAddr(vdp, 0x80 | (uint8_t)reg);
}

/*
 * Set current VRAM address for reading
 */
inline static void tmsSetAddressRead(TMS9918A* vdp, uint16_t addr)
{
	tmsWriteAddr(vdp, addr & 0x00ff);
	tmsWriteAddr(vdp, ((addr & 0xff00) >> 8));
}

/*
 * Set current VRAM address for writing
 */
inline static void tmsSetAddressWrite(TMS9918A* vdp, uint16_t addr)
{
	tmsSetAddressRead(vdp, addr | 0x4000);
}

/*
 * Write a series of bytes to the VRAM
 */
inline static void tmsWriteBytes(TMS9918A* vdp, const uint8_t* bytes, size_t numBytes)
{
	for (size_t i = 0; i < numBytes; ++i)
	{
		tmsWriteData(vdp, bytes[i]);
	}
}

/*
 * Write a series of bytes to the VRAM
 */
inline static void tmsWriteByteRpt(TMS9918A* vdp, uint8_t byte, size_t rpt)
{
	for (size_t i = 0; i < rpt; ++i)
	{
		tmsWriteData(vdp, byte);
	}
}


/*
 * Write a series of chars to the VRAM
 */
inline static void tmsWriteString(TMS9918A* vdp, const char* str)
{
	size_t len = strlen(str);
	for (size_t i = 0; i < len; ++i)
	{
		tmsWriteData(vdp, str[i]);
	}
}

/*
 * Return a colur byte consisting of foreground and background colors
 */
inline static uint8_t tmsFgBgColor(TMS9918A_COLOR fg, TMS9918A_COLOR bg)
{
	return (uint8_t)((uint8_t)fg << 4) | (uint8_t)bg;
}

/*
 * Set name table address
 */
inline static void tmsSetNameTableAddr(TMS9918A* vdp, uint16_t addr)
{
	tmsWriteRegisterValue(vdp, TMS_REG_NAME_TABLE, addr >> 10);
}

/*
 * Set color table address
 */
inline static void tmsSetColorTableAddr(TMS9918A* vdp, uint16_t addr)
{
	tmsWriteRegisterValue(vdp, TMS_REG_COLOR_TABLE, (uint8_t)(addr >> 6));
}

/*
 * Set pattern table address
 */
inline static void tmsSetPatternTableAddr(TMS9918A* vdp, uint16_t addr)
{
	tmsWriteRegisterValue(vdp, TMS_REG_PATTERN_TABLE, addr >> 11);
}

/*
 * Set sprite attribute table address
 */
inline static void tmsSetSpriteAttrTableAddr(TMS9918A* vdp, uint16_t addr)
{
	tmsWriteRegisterValue(vdp, TMS_REG_SPRITE_ATTR_TABLE, (uint8_t)(addr >> 7));
}

/*
 * Set sprite pattern table address
 */
inline static void tmsSetSpritePattTableAddr(TMS9918A* vdp, uint16_t addr)
{
	tmsWriteRegisterValue(vdp, TMS_REG_SPRITE_PATT_TABLE, addr >> 11);
}

/*
 * Set foreground (text mode) and background colors
 */
inline static void tmsSetFgBgColor(TMS9918A* vdp, TMS9918A_COLOR fg, TMS9918A_COLOR bg)
{
	tmsWriteRegisterValue(vdp, TMS_REG_FG_BG_COLOR, tmsFgBgColor(fg, bg));
}


/*
 * Initialise for Graphics I mode
 */
void tmsInitialiseGfxI(TMS9918A* vdp);

/*
 * Initialise for Graphics II mode
 */
void tmsInitialiseGfxII(TMS9918A* vdp);

/*
 * Initialise for Text mode
 */
void tmsInitialiseText(TMS9918A* vdp);

// Device/bus interface
struct TMS9918ADevice_s {
	TMS9918A*		vdp;
	uint32_t		frameBuffer[TMS9918_DISPLAY_PIXELS];
	float			unusedTime;
	int				currentFramePixels;
	uint8_t			scanlineBuffer[TMS9918_DISPLAY_WIDTH];
	uint8_t			irq;
	SDL_Window*		window;
	SDL_Renderer*	renderer;
	SDL_Texture*	texture;
};
extern struct TMS9918ADevice_s tmsVDP;
void initTMS9918A(void);
void destroyTMS9918A(void);
void resetTMS9918A(void);
uint8_t inTMS9918A(uint16_t portnum);
void outTMS9918A(uint16_t portnum, uint8_t value);
void renderTMS9918A();
void tickTMS9918A(uint32_t deltaTicks, float deltaTime);

#endif