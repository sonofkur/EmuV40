#include "tms9918a.h"
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "ports.h"

uint32_t tmsPalette[] = {
  0x00000000, /* transparent */
  0x000000ff, /* black */
  0x21c942ff, /* medium green */
  0x5edc78ff, /* light green */
  0x5455edff, /* dark blue */
  0x7d75fcff, /* light blue */
  0xd3524dff, /* dark red */
  0x43ebf6ff, /* cyan */
  0xfd5554ff, /* medium red */
  0xff7978ff, /* light red */
  0xd3c153ff, /* dark yellow */
  0xe5ce80ff, /* light yellow */
  0x21b03cff, /* dark green */
  0xc95bbaff, /* magenta */
  0xccccccff, /* grey */
  0xffffffff  /* white */
};

struct TMS9918ADevice_s tmsVDP;

void initTMS9918A(void) {
	memset((void*)&tmsVDP, 0, sizeof(tmsVDP));
	set_port_write_redirector(0xA0, 0xA1, &outTMS9918A);
	set_port_read_redirector(0xA0, 0xA1, &inTMS9918A);

	tmsVDP.vdp = tmsNew();
	tmsVDP.unusedTime = 0.0f;
	tmsVDP.currentFramePixels = 0;
	memset(tmsVDP.frameBuffer, 0, sizeof(tmsVDP.frameBuffer));
	memset(tmsVDP.scanlineBuffer, 6, sizeof(tmsVDP.scanlineBuffer));

	uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"best");
	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1"); // Grabs keyboard shortcuts from the system during window grab
	SDL_CreateWindowAndRenderer((int)(TMS9918_DISPLAY_WIDTH), (int)(TMS9918_DISPLAY_HEIGHT), window_flags, &tmsVDP.window, &tmsVDP.renderer);
	SDL_SetWindowResizable(tmsVDP.window, true);
	SDL_RenderSetLogicalSize(tmsVDP.renderer, (int)(TMS9918_DISPLAY_WIDTH), TMS9918_DISPLAY_HEIGHT);

	tmsVDP.texture = SDL_CreateTexture(tmsVDP.renderer,
		SDL_PIXELFORMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING,
		TMS9918_DISPLAY_WIDTH, TMS9918_DISPLAY_HEIGHT);

	SDL_SetWindowTitle(tmsVDP.window, "V40 Emu");
}

void destroyTMS9918A(void) {
	if (tmsVDP.vdp != NULL) {
		tmsDestroy(tmsVDP.vdp);
	}

	/*free(tmsVDP.vdp);
	tmsVDP.vdp = NULL;*/

	SDL_DestroyTexture(tmsVDP.texture);
	tmsVDP.texture = NULL;
}

void resetTMS9918A(void) {
	tmsReset(tmsVDP.vdp);
}

uint8_t inTMS9918A(uint16_t portnum) {
	uint8_t data;
	if (portnum & 1) {
		data = tmsReadStatus(tmsVDP.vdp);
	}
	else {
		data = tmsReadData(tmsVDP.vdp);
	}
	return data;
}

void outTMS9918A(uint16_t portnum, uint8_t value) {
	if (portnum & 1) {
		tmsWriteAddr(tmsVDP.vdp, value);
	}
	else {
		tmsWriteData(tmsVDP.vdp, value);
	}
}

void renderTMS9918A() {
	if (tmsVDP.vdp)
	{
		void* pixels = NULL;
		int pitch = 0;
		SDL_LockTexture(tmsVDP.texture, NULL, &pixels, &pitch);
		memcpy(pixels, tmsVDP.frameBuffer, sizeof(tmsVDP.frameBuffer));
		SDL_UnlockTexture(tmsVDP.texture);
	}
}

void tickTMS9918A(uint32_t deltaTicks, float deltaTime) {
	/* determine portion of frame to render */
	deltaTime += tmsVDP.unusedTime;

	/* how many pixels are we rendering? */
	float thisStepTotalPixelsFlt = 0.0f;
	tmsVDP.unusedTime = modff(deltaTime / (float)TMS9918_PIXEL_TIME, &thisStepTotalPixelsFlt) * TMS9918_PIXEL_TIME;
	int thisStepTotalPixels = (uint32_t)thisStepTotalPixelsFlt;

	/* if we haven't reached the minimum, accumulate time for the next call and return */
	if (thisStepTotalPixels < TMS9918_TICK_MIN_PIXELS)
	{
		tmsVDP.unusedTime += thisStepTotalPixels * TMS9918_PIXEL_TIME;
		return;
	}

	/* we only render the end end of a frame. if we need to go further, accumulate the time for the next call */
	if (tmsVDP.currentFramePixels + thisStepTotalPixels >= TMS9918_DISPLAY_PIXELS)
	{
		tmsVDP.unusedTime += ((tmsVDP.currentFramePixels + thisStepTotalPixels) - TMS9918_DISPLAY_PIXELS) * TMS9918_PIXEL_TIME;
		thisStepTotalPixels = TMS9918_DISPLAY_PIXELS - tmsVDP.currentFramePixels;
	}

	/* get the background color for this run of pixels */
	uint8_t bgColor = tmsRegValue(tmsVDP.vdp, TMS_REG_7) & 0x0f;

	//bgColor = (++c) & 0x0f;  /* for testing */
	int firstPix = 1;
	uint32_t* fbPtr = tmsVDP.frameBuffer + tmsVDP.currentFramePixels;

	int tmsRow = 0;

	/* iterate over the pixels we need to update in this call */
	for (int p = 0; p < thisStepTotalPixels; ++p)
	{
		div_t currentPos = div((int)tmsVDP.currentFramePixels, (int)TMS9918_DISPLAY_WIDTH);

		int currentRow = currentPos.quot;
		int currentCol = currentPos.rem;

		/* if this is the first pixel or the first pixel of a new row, update the scanline buffer */
		if (firstPix || currentCol == 0)
		{
			tmsRow = currentRow - TMS9918_BORDER_Y;
			memset(tmsVDP.scanlineBuffer, bgColor, sizeof(tmsVDP.scanlineBuffer));
			if (tmsRow >= 0 && tmsRow < TMS9918_PIXELS_Y)
			{
				tmsScanline(tmsVDP.vdp, (uint8_t)tmsRow, tmsVDP.scanlineBuffer + TMS9918_BORDER_X);
			}

			firstPix = 0;
		}

		/* update the frame buffer pixel from the scanline pixel */
		*(fbPtr++) = tmsPalette[tmsVDP.scanlineBuffer[currentCol]];

		/* if we're at the end of the main tms9918 frame, trigger an interrupt */
		if (++tmsVDP.currentFramePixels == (TMS9918_DISPLAY_WIDTH * (TMS9918_DISPLAY_HEIGHT - TMS9918_BORDER_Y)))
		{
			if (tmsRegValue(tmsVDP.vdp, TMS_REG_1) & 0x20)
			{
				//hbc56Interrupt(tmsDevice->irq, INTERRUPT_RAISE);
			}
		}
	}

	/* reset pixel count if frame finished */
	if (tmsVDP.currentFramePixels >= TMS9918_DISPLAY_PIXELS) tmsVDP.currentFramePixels = 0;
}

/* Function:  tmsMode
 * ----------------------------------------
 * return the current display mode
 */
static TMS9918A_MODE tmsMode(TMS9918A* vdp)
{
	if (vdp->registers[TMS_REG_0] & TMS_R0_MODE_GRAPHICS_II)
	{
		return TMS_MODE_GRAPHICS_II;
	}

	/* MC and TEX bits 3 and 4. Shift to bits 0 and 1 to determine a value (0, 1 or 2) */
	switch ((vdp->registers[TMS_REG_1] & (TMS_R1_MODE_MULTICOLOR | TMS_R1_MODE_TEXT)) >> 3)
	{
	case 0:
		return TMS_MODE_GRAPHICS_I;

	case 1:
		return TMS_MODE_MULTICOLOR;

	case 2:
		return TMS_MODE_TEXT;
	}
	return TMS_MODE_GRAPHICS_I;
}


/* Function:  tmsSpriteSize
 * ----------------------------------------
 * sprite size (8 or 16)
 */
static inline uint8_t tmsSpriteSize(TMS9918A* vdp)
{
	return vdp->registers[TMS_REG_1] & TMS_R1_SPRITE_16 ? 16 : 8;
}

/* Function:  tmsSpriteMagnification
 * ----------------------------------------
 * sprite size (0 = 1x, 1 = 2x)
 */
static inline bool tmsSpriteMag(TMS9918A* vdp)
{
	return vdp->registers[TMS_REG_1] & TMS_R1_SPRITE_MAG2;
}

/* Function:  tmsNameTableAddr
 * ----------------------------------------
 * name table base address
 */
static inline uint16_t tmsNameTableAddr(TMS9918A* vdp)
{
	return (vdp->registers[TMS_REG_NAME_TABLE] & 0x0f) << 10;
}

/* Function:  tmsColorTableAddr
 * ----------------------------------------
 * color table base address
 */
static inline uint16_t tmsColorTableAddr(TMS9918A* vdp)
{
	const uint8_t mask = (vdp->mode == TMS_MODE_GRAPHICS_II) ? 0x80 : 0xff;

	return (vdp->registers[TMS_REG_COLOR_TABLE] & mask) << 6;
}

/* Function:  tmsPatternTableAddr
 * ----------------------------------------
 * pattern table base address
 */
static inline uint16_t tmsPatternTableAddr(TMS9918A* vdp)
{
	const uint8_t mask = (vdp->mode == TMS_MODE_GRAPHICS_II) ? 0x04 : 0x07;

	return (vdp->registers[TMS_REG_PATTERN_TABLE] & mask) << 11;
}

/* Function:  tmsSpriteAttrTableAddr
 * ----------------------------------------
 * sprite attribute table base address
 */
static inline uint16_t tmsSpriteAttrTableAddr(TMS9918A* vdp)
{
	return (vdp->registers[TMS_REG_SPRITE_ATTR_TABLE] & 0x7f) << 7;
}

/* Function:  tmsSpritePatternTableAddr
 * ----------------------------------------
 * sprite pattern table base address
 */
static inline uint16_t tmsSpritePatternTableAddr(TMS9918A* vdp)
{
	return (vdp->registers[TMS_REG_SPRITE_PATT_TABLE] & 0x07) << 11;
}

/* Function:  tmsBgColor
 * ----------------------------------------
 * background color
 */
static inline TMS9918A_COLOR tmsMainBgColor(TMS9918A* vdp)
{
	return vdp->registers[TMS_REG_FG_BG_COLOR] & 0x0f;
}

/* Function:  tmsFgColor
 * ----------------------------------------
 * foreground color
 */
static inline TMS9918A_COLOR tmsMainFgColor(TMS9918A* vdp)
{
	const TMS9918A_COLOR c = (TMS9918A_COLOR)(vdp->registers[TMS_REG_FG_BG_COLOR] >> 4);
	return c == TMS_TRANSPARENT ? tmsMainBgColor(vdp) : c;
}

/* Function:  tmsFgColor
 * ----------------------------------------
 * foreground color
 */
static inline TMS9918A_COLOR tmsFgColor(TMS9918A* vdp, uint8_t colorByte)
{
	const TMS9918A_COLOR c = (TMS9918A_COLOR)(colorByte >> 4);
	return c == TMS_TRANSPARENT ? tmsMainBgColor(vdp) : c;
}

/* Function:  tmsBgColor
 * ----------------------------------------
 * background color
 */
static inline TMS9918A_COLOR tmsBgColor(TMS9918A* vdp, uint8_t colorByte)
{
	const TMS9918A_COLOR c = (TMS9918A_COLOR)(colorByte & 0x0f);
	return c == TMS_TRANSPARENT ? tmsMainBgColor(vdp) : c;
}

TMS9918A* tmsNew() {
	TMS9918A* vdp = (TMS9918A*)malloc(sizeof(TMS9918A));
	if (vdp != NULL)
	{
		tmsReset(vdp);
	}

	return vdp;
}

void tmsReset(TMS9918A* vdp) {
	if (vdp)
	{
		vdp->regWriteStage0Value = 0;
		vdp->currentAddress = 0;
		vdp->regWriteStage = 0;
		vdp->status = 0;
		vdp->readAheadBuffer = 0;
		memset(vdp->registers, 0, sizeof(vdp->registers));

		/* ram intentionally left in unknown state */

		vdp->mode = tmsMode(vdp);
	}
}

void tmsDestroy(TMS9918A* vdp) {
	if (vdp)
	{
		free(vdp);
	}
}

void tmsWriteAddr(TMS9918A* vdp, uint8_t data) {
	if (vdp == NULL) return;

	if (vdp->regWriteStage == 0)
	{
		/* first stage byte - either an address LSB or a register value */

		vdp->regWriteStage0Value = data;
		vdp->regWriteStage = 1;
	}
	else
	{
		/* second byte - either a register number or an address MSB */

		if (data & 0x80) /* register */
		{
			vdp->registers[data & 0x07] = vdp->regWriteStage0Value;

			vdp->mode = tmsMode(vdp);
		}
		else /* address */
		{
			vdp->currentAddress = vdp->regWriteStage0Value | ((data & 0x3f) << 8);
			if ((data & 0x40) == 0)
			{
				vdp->readAheadBuffer = vdp->vram[(vdp->currentAddress++) & VRAM_MASK];
			}
		}
		vdp->regWriteStage = 0;
	}
}

void tmsWriteData(TMS9918A* vdp, uint8_t data) {
	if (vdp == NULL) return;

	vdp->regWriteStage = 0;
	vdp->readAheadBuffer = data;
	vdp->vram[(vdp->currentAddress++) & VRAM_MASK] = data;
}

uint8_t tmsReadStatus(TMS9918A* vdp) {
	if (vdp == NULL) return 0;

	const uint8_t tmpStatus = vdp->status;
	vdp->status = 0;
	vdp->regWriteStage = 0;
	return tmpStatus;
}

uint8_t tmsReadData(TMS9918A* vdp) {
	if (vdp == NULL) return 0;

	vdp->regWriteStage = 0;
	uint8_t currentValue = vdp->readAheadBuffer;
	vdp->readAheadBuffer = vdp->vram[(vdp->currentAddress++) & VRAM_MASK];
	return currentValue;
}

uint8_t tmsReadDataNoInc(TMS9918A* vdp) {
	if (vdp == NULL) return 0;

	return vdp->readAheadBuffer;
}

uint8_t tmsRegValue(TMS9918A* vdp, TMS9918A_REGISTER reg) {
	if (vdp == NULL)
		return 0;

	return vdp->registers[reg & 0x07];
}

void tmsWriteRegValue(TMS9918A* vdp, TMS9918A_REGISTER reg, uint8_t value) {
	if (vdp != NULL)
	{
		vdp->registers[reg & 0x07] = value;
		vdp->mode = tmsMode(vdp);
	}
}

uint8_t tmsVramValue(TMS9918A* vdp, uint16_t addr) {
	if (vdp == NULL)
		return 0;

	return vdp->vram[addr & VRAM_MASK];
}

bool tmsDisplayEnabled(TMS9918A* vdp) {
	if (vdp == NULL)
		return false;

	return vdp->registers[TMS_REG_1] & TMS_R1_DISP_ACTIVE;
}

TMS9918A_MODE tmsDisplayMode(TMS9918A* vdp) {
	return vdp->mode;
}

/* Function:  vrEmuTms9918OutputSprites
 * ----------------------------------------
 * Output Sprites to a scanline
 */
static void tmsOutputSprites(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X])
{
	const bool spriteMag = tmsSpriteMag(vdp);
	const bool sprite16 = tmsSpriteSize(vdp) == 16;
	const uint8_t spriteSize = tmsSpriteSize(vdp);
	const uint8_t spriteSizePx = spriteSize * (spriteMag + 1);
	const uint16_t spriteAttrTableAddr = tmsSpriteAttrTableAddr(vdp);
	const uint16_t spritePatternAddr = tmsSpritePatternTableAddr(vdp);

	uint8_t spritesShown = 0;

	if (y == 0)
	{
		vdp->status = 0;
	}

	uint8_t* spriteAttr = vdp->vram + spriteAttrTableAddr;
	for (uint8_t spriteIdx = 0; spriteIdx < MAX_SPRITES; ++spriteIdx)
	{
		int16_t yPos = spriteAttr[SPRITE_ATTR_Y];

		/* stop processing when yPos == LAST_SPRITE_YPOS */
		if (yPos == LAST_SPRITE_YPOS)
		{
			if ((vdp->status & STATUS_5S) == 0)
			{
				vdp->status |= spriteIdx;
			}
			break;
		}

		/* check if sprite position is in the -31 to 0 range and move back to top */
		if (yPos > 0xe0)
		{
			yPos -= 256;
		}

		/* first row is YPOS -1 (0xff). 2nd row is YPOS 0 */
		yPos += 1;

		int16_t pattRow = y - yPos;
		if (spriteMag)
		{
			pattRow >>= 1;  // this needs to be a shift because -1 / 2 becomes 0. Bad.
		}

		/* check if sprite is visible on this line */
		if (pattRow < 0 || pattRow >= spriteSize)
		{
			spriteAttr += SPRITE_ATTR_BYTES;
			continue;
		}

		if (spritesShown == 0)
		{
			int* rsbInt = (int*)vdp->rowSpriteBits;
			int* end = rsbInt + sizeof(vdp->rowSpriteBits) / sizeof(int);

			while (rsbInt < end)
			{
				*rsbInt++ = 0;
			}
		}

		const uint8_t spriteColor = spriteAttr[SPRITE_ATTR_COLOR] & 0x0f;

		/* have we exceeded the scanline sprite limit? */
		if (++spritesShown > MAX_SCANLINE_SPRITES)
		{
			if ((vdp->status & STATUS_5S) == 0)
			{
				vdp->status |= STATUS_5S | spriteIdx;
			}
			break;
		}

		/* sprite is visible on this line */
		const uint8_t pattIdx = spriteAttr[SPRITE_ATTR_NAME];
		const uint16_t pattOffset = spritePatternAddr + pattIdx * PATTERN_BYTES + (uint16_t)pattRow;

		const int16_t earlyClockOffset = (spriteAttr[SPRITE_ATTR_COLOR] & 0x80) ? -32 : 0;
		const int16_t xPos = (int16_t)(spriteAttr[SPRITE_ATTR_X]) + earlyClockOffset;

		int8_t pattByte = vdp->vram[pattOffset];
		uint8_t screenBit = 0, pattBit = 0;

		int16_t endXPos = xPos + spriteSizePx;
		if (endXPos >= TMS9918_PIXELS_X)
		{
			endXPos = TMS9918_PIXELS_X;
		}

		for (int16_t screenX = xPos; screenX < endXPos; ++screenX, ++screenBit)
		{
			if (screenX >= 0)
			{
				if (pattByte < 0)
				{
					if (spriteColor != TMS_TRANSPARENT && vdp->rowSpriteBits[screenX] < 2)
					{
						pixels[screenX] = spriteColor;
					}

					/* we still process transparent sprites, since
					   they're used in 5S and collision checks */
					if (vdp->rowSpriteBits[screenX])
					{
						vdp->status |= STATUS_COL;
					}
					else
					{
						vdp->rowSpriteBits[screenX] = spriteColor + 1;
					}
				}
			}

			/* next pattern bit if non-magnified or if odd screen bit */
			if (!spriteMag || (screenBit & 0x01))
			{
				pattByte <<= 1;
				if (++pattBit == GRAPHICS_CHAR_WIDTH && sprite16) /* from A -> C or B -> D of large sprite */
				{
					pattBit = 0;
					pattByte = vdp->vram[pattOffset + PATTERN_BYTES * 2];
				}
			}
		}
		spriteAttr += SPRITE_ATTR_BYTES;
	}

}


/* Function:  vrEmuTms9918GraphicsIScanLine
 * ----------------------------------------
 * generate a Graphics I mode scanline
 */
static void tmsGraphicsIScanline(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X])
{
	const uint8_t tileY = y >> 3;   /* which name table row (0 - 23) */
	const uint8_t pattRow = y & 0x07;  /* which pattern row (0 - 7) */

	/* address in name table at the start of this row */
	const uint16_t rowNamesAddr = tmsNameTableAddr(vdp) + tileY * GRAPHICS_NUM_COLS;

	const uint8_t* patternTable = vdp->vram + tmsPatternTableAddr(vdp);
	const uint8_t* colorTable = vdp->vram + tmsColorTableAddr(vdp);

	/* iterate over each tile in this row */
	for (uint8_t tileX = 0; tileX < GRAPHICS_NUM_COLS; ++tileX)
	{
		const uint8_t pattIdx = vdp->vram[rowNamesAddr + tileX];
		uint8_t pattByte = patternTable[pattIdx * PATTERN_BYTES + pattRow];
		const uint8_t colorByte = colorTable[pattIdx / GFXI_COLOR_GROUP_SIZE];

		const uint8_t fgColor = tmsFgColor(vdp, colorByte);
		const uint8_t bgColor = tmsBgColor(vdp, colorByte);

		/* iterate over each bit of this pattern byte */
		for (uint8_t pattBit = 0; pattBit < GRAPHICS_CHAR_WIDTH; ++pattBit)
		{
			const bool pixelBit = pattByte & 0x80;
			*(pixels++) = pixelBit ? fgColor : bgColor;
			pattByte <<= 1;
		}
	}

	tmsOutputSprites(vdp, y, pixels - TMS9918_PIXELS_X);
}

/* Function:  vrEmuTms9918GraphicsIIScanLine
 * ----------------------------------------
 * generate a Graphics II mode scanline
 */
static void tmsGraphicsIIScanline(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X])
{
	const uint8_t tileY = y >> 3;   /* which name table row (0 - 23) */
	const uint8_t pattRow = y & 0x07;  /* which pattern row (0 - 7) */

	/* address in name table at the start of this row */
	const uint16_t rowNamesAddr = tmsNameTableAddr(vdp) + tileY * GRAPHICS_NUM_COLS;

	/* the datasheet says the lower bits of the color and pattern tables must
	   be all 1's for graphics II mode. however, the lowest 2 bits of the
	   pattern address are used to determine if pages 2 & 3 come from page 0
	   or not. Similarly, the lowest 6 bits of the color table register are
	   used as an and mask with the nametable  index */
	const uint8_t nameMask = ((vdp->registers[TMS_REG_COLOR_TABLE] & 0x7f) << 3) | 0x07;

	const uint16_t pageThird = ((tileY & 0x18) >> 3)
		& (vdp->registers[TMS_REG_PATTERN_TABLE] & 0x03); /* which page? 0-2 */
	const uint16_t pageOffset = pageThird << 11; /* offset (0, 0x800 or 0x1000) */

	const uint8_t* patternTable = vdp->vram + tmsPatternTableAddr(vdp) + pageOffset;
	const uint8_t* colorTable = vdp->vram + tmsColorTableAddr(vdp) + (pageOffset
		& ((vdp->registers[TMS_REG_COLOR_TABLE] & 0x60) << 6));

	/* iterate over each tile in this row */
	for (uint8_t tileX = 0; tileX < GRAPHICS_NUM_COLS; ++tileX)
	{
		uint8_t pattIdx = vdp->vram[rowNamesAddr + tileX] & nameMask;

		const size_t pattRowOffset = pattIdx * PATTERN_BYTES + pattRow;
		const uint8_t pattByte = patternTable[pattRowOffset];
		const uint8_t colorByte = colorTable[pattRowOffset];

		const TMS9918A_COLOR fgColor = tmsFgColor(vdp, colorByte);
		const TMS9918A_COLOR bgColor = tmsBgColor(vdp, colorByte);

		/* iterate over each bit of this pattern byte */
		for (uint8_t pattBit = 0; pattBit < GRAPHICS_CHAR_WIDTH; ++pattBit)
		{
			const bool pixelBit = (pattByte << pattBit) & 0x80;
			pixels[tileX * GRAPHICS_CHAR_WIDTH + pattBit] = (uint8_t)(pixelBit ? fgColor : bgColor);
		}
	}

	tmsOutputSprites(vdp, y, pixels);
}

/* Function:  vrEmuTms9918TextScanLine
 * ----------------------------------------
 * generate a Text mode scanline
 */
static void tmsGraphicsTextScanline(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X])
{
	const uint8_t tileY = y >> 3;   /* which name table row (0 - 23) */
	const uint8_t pattRow = y & 0x07;  /* which pattern row (0 - 7) */

	/* address in name table at the start of this row */
	const uint16_t rowNamesAddr = tmsNameTableAddr(vdp) + tileY * TEXT_NUM_COLS;
	const uint8_t* patternTable = vdp->vram + tmsPatternTableAddr(vdp);

	const TMS9918A_COLOR bgColor = tmsMainBgColor(vdp);
	const TMS9918A_COLOR fgColor = tmsMainFgColor(vdp);

	/* fill the first and last 8 pixels with bg color */
	memset(pixels, bgColor, TEXT_PADDING_PX);
	memset(pixels + TMS9918_PIXELS_X - TEXT_PADDING_PX, bgColor, TEXT_PADDING_PX);

	for (uint8_t tileX = 0; tileX < TEXT_NUM_COLS; ++tileX)
	{
		const uint8_t pattIdx = vdp->vram[rowNamesAddr + tileX];
		const uint8_t pattByte = patternTable[pattIdx * PATTERN_BYTES + pattRow];

		for (uint8_t pattBit = 0; pattBit < TEXT_CHAR_WIDTH; ++pattBit)
		{
			bool pixelBit = (pattByte << pattBit) & 0x80;
			pixels[TEXT_PADDING_PX + tileX * TEXT_CHAR_WIDTH + pattBit] = (uint8_t)(pixelBit ? fgColor : bgColor);
		}
	}
}

/* Function:  vrEmuTms9918MulticolorScanLine
 * ----------------------------------------
 * generate a Multicolor mode scanline
 */
static void tmsGraphicsMulticolorScanline(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X])
{
	const uint8_t tileY = y >> 3;
	const uint8_t pattRow = ((y / 4) & 0x01) + (tileY & 0x03) * 2;

	const uint16_t namesAddr = tmsNameTableAddr(vdp) + tileY * GRAPHICS_NUM_COLS;
	const uint8_t* patternTable = vdp->vram + tmsPatternTableAddr(vdp);

	for (uint8_t tileX = 0; tileX < GRAPHICS_NUM_COLS; ++tileX)
	{
		const uint8_t pattIdx = vdp->vram[namesAddr + tileX];
		const uint8_t colorByte = patternTable[pattIdx * PATTERN_BYTES + pattRow];

		memset(pixels + tileX * 8, tmsFgColor(vdp, colorByte), 4);
		memset(pixels + tileX * 8 + 4, tmsBgColor(vdp, colorByte), 4);
	}

	tmsOutputSprites(vdp, y, pixels);
}

void tmsScanline(TMS9918A* vdp, uint8_t y, uint8_t pixels[TMS9918_PIXELS_X]) {
	if (vdp == NULL)
		return;

	if (!tmsDisplayEnabled(vdp) || y >= TMS9918_PIXELS_Y)
	{
		memset(pixels, tmsMainBgColor(vdp), TMS9918_PIXELS_X);
		return;
	}

	switch (vdp->mode)
	{
	case TMS_MODE_GRAPHICS_I:
		tmsGraphicsIScanline(vdp, y, pixels);
		break;

	case TMS_MODE_GRAPHICS_II:
		tmsGraphicsIIScanline(vdp, y, pixels);
		break;

	case TMS_MODE_TEXT:
		tmsGraphicsTextScanline(vdp, y, pixels);
		break;

	case TMS_MODE_MULTICOLOR:
		tmsGraphicsMulticolorScanline(vdp, y, pixels);
		break;
	}

	if (y == TMS9918_PIXELS_Y - 1 && (vdp->registers[1] & TMS_R1_INT_ENABLE))
	{
		vdp->status |= STATUS_INT;
	}
}

// UTILS
static void clearTmsRam(TMS9918A* vdp)
{
	tmsSetAddressWrite(vdp, 0x0000);
	tmsWriteByteRpt(vdp, 0x00, 0x4000);

	tmsSetAddressWrite(vdp, TMS_DEFAULT_VRAM_SPRITE_ATTR_ADDRESS);
	for (int i = 0; i < 32; ++i)
	{
		tmsWriteData(vdp, LAST_SPRITE_YPOS);
		tmsWriteData(vdp, 0);
		tmsWriteData(vdp, 0);
		tmsWriteData(vdp, 0);
	}
}

void tmsInitialiseGfxI(TMS9918A* vdp)
{
	tmsWriteRegisterValue(vdp, TMS_REG_0, TMS_R0_EXT_VDP_DISABLE | TMS_R0_MODE_GRAPHICS_I);
	tmsWriteRegisterValue(vdp, TMS_REG_1, TMS_R1_RAM_16K | TMS_R1_MODE_GRAPHICS_I | TMS_R1_RAM_16K | TMS_R1_DISP_ACTIVE | TMS_R1_INT_ENABLE);
	tmsSetNameTableAddr(vdp, TMS_DEFAULT_VRAM_NAME_ADDRESS);
	tmsSetColorTableAddr(vdp, TMS_DEFAULT_VRAM_COLOR_ADDRESS);
	tmsSetPatternTableAddr(vdp, TMS_DEFAULT_VRAM_PATT_ADDRESS);
	tmsSetSpriteAttrTableAddr(vdp, TMS_DEFAULT_VRAM_SPRITE_ATTR_ADDRESS);
	tmsSetSpritePattTableAddr(vdp, TMS_DEFAULT_VRAM_SPRITE_PATT_ADDRESS);
	tmsSetFgBgColor(vdp, TMS_BLACK, TMS_CYAN);

	clearTmsRam(vdp);
}



void tmsInitialiseGfxII(TMS9918A* vdp)
{
	tmsWriteRegisterValue(vdp, TMS_REG_0, TMS_R0_EXT_VDP_DISABLE | TMS_R0_MODE_GRAPHICS_II);
	tmsWriteRegisterValue(vdp, TMS_REG_1, TMS_R1_RAM_16K | TMS_R1_MODE_GRAPHICS_II | TMS_R1_RAM_16K | TMS_R1_DISP_ACTIVE | TMS_R1_INT_ENABLE);
	tmsSetNameTableAddr(vdp, TMS_DEFAULT_VRAM_NAME_ADDRESS);

	/* in Graphics II, Registers 3 and 4 work differently
	 *
	 * reg3 - Color table
	 *   0x7f = 0x0000
	 *   0xff = 0x2000
	 *
	 * reg4 - Pattern table
	 *  0x03 = 0x0000
	 *  0x07 = 0x2000
	 */

	tmsWriteRegisterValue(vdp, TMS_REG_COLOR_TABLE, 0x7f);
	tmsWriteRegisterValue(vdp, TMS_REG_PATTERN_TABLE, 0x07);

	tmsSetSpriteAttrTableAddr(vdp, TMS_DEFAULT_VRAM_SPRITE_ATTR_ADDRESS);
	tmsSetSpritePattTableAddr(vdp, TMS_DEFAULT_VRAM_SPRITE_PATT_ADDRESS);
	tmsSetFgBgColor(vdp, TMS_BLACK, TMS_CYAN);

	clearTmsRam(vdp);

	tmsSetAddressWrite(vdp, TMS_DEFAULT_VRAM_NAME_ADDRESS);
	for (int i = 0; i < 768; ++i)
	{
		tmsWriteData(vdp, i & 0xff);
	}

}

void tmsInitialiseText(TMS9918A* vdp) {
	tmsWriteRegisterValue(vdp, TMS_REG_0, TMS_R0_EXT_VDP_DISABLE | TMS_R0_MODE_TEXT);
	tmsWriteRegisterValue(vdp, TMS_REG_1, TMS_R1_RAM_16K | TMS_R1_MODE_TEXT | TMS_R1_RAM_16K | TMS_R1_DISP_ACTIVE | TMS_R1_INT_ENABLE);
	tmsSetNameTableAddr(vdp, 0x0800);
	//tmsSetColorTableAddr(vdp, TMS_DEFAULT_VRAM_COLOR_ADDRESS);
	tmsSetPatternTableAddr(vdp, 0x0000);
	//tmsSetSpriteAttrTableAddr(vdp, TMS_DEFAULT_VRAM_SPRITE_ATTR_ADDRESS);
	//tmsSetSpritePattTableAddr(vdp, TMS_DEFAULT_VRAM_SPRITE_PATT_ADDRESS);
	tmsSetFgBgColor(vdp, TMS_WHITE, TMS_CYAN);

	clearTmsRam(vdp);
}