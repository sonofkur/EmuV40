#ifndef F18A_H
#define F18A_H

#include <SDL.h>
#include <stdbool.h>

#define VDP_DISPLAY_WIDTH	256 + 16
#define VDP_DISPLAY_HEIGHT	192 + 16
#define REDRAW_LINES		262
#define VDPINT				((VDPS&VDPS_INT) && (VDPREG[1]&0x20))	// VDP hardware interrupt pin and mask
#define GETPALETTEVALUE(n)	F18APalette[(bF18AActive?((VDPREG[0x18]&03)<<4)+(n) : (n))]

#define CLOCK_MHZ 8000000

// note these actual timings as calculated from the VDP datasheet
// calculated it as 62.6 - but a long term interrupt count test gave 59.9
// so back to 60
#define HZ60 60
// calculated it as 50.23
#define HZ50 50
#define DEFAULT_60HZ_CPF (CLOCK_MHZ/HZ60)
#define DEFAULT_50HZ_CPF (CLOCK_MHZ/HZ50)
#define SLOW_CPF (10)
#define SPEECHRATE 8000	
#define SPEECHBUFFER 16000
#define MAX_BREAKPOINTS 10
#define MAXROMSPERCART	32
#define MAXUSERCARTS 1000
#define MAX_MRU 10

// replaces the old dual throttle
#define THROTTLE_NONE 9999
#define THROTTLE_SLOW -1
#define THROTTLE_NORMAL 0
#define THROTTLE_OVERDRIVE 1
#define THROTTLE_SYSTEMMAXIMUM 2

// note: not enough to change this - you also need to change the
// paging/padding calculation in the loader!
#define MAX_BANKSWITCH_SIZE (512*1024*1024)

// VDP status flags
#define VDPS_INT	0x80
#define VDPS_5SPR	0x40
#define VDPS_SCOL	0x20

char VDPGetChar(int x, int y, int width, int height);
void VDPinit(void);
void VDPshutdown(void);
void VDPreset(bool isCold);
//HRESULT InitDirectDraw(HWND hWnd);
void VDPdisplay(int scanline);
void updateVDP(int cycleCount);
void vdpForceFrame();
int  gettables(int isLayer2);
void draw_debug(void);
void VDPgraphics(int scanline, int isLayer2);
void VDPgraphicsII(int scanline, int isLayer2);
void VDPtext(int scanline, int isLayer2);
void VDPtextII(int scanline, int isLayer2);
void VDPtext80(int scanline, int isLayer2);
void VDPillegal(int scanline, int isLayer2);
void VDPmulticolor(int scanline, int isLayer2);
void VDPmulticolorII(int scanline, int isLayer2);
unsigned char getF18AStatus();
void renderBML(int y);
void doBlit(void);
void RenderFont(void);
void DrawSprites(int scanline);
void pixel(int x, int y, int col);
void pixel80(int x, int y, int col);
void bigpixel(int x, int y, int col);
void spritepixel(int x, int y, int c);
// Added by RasmusM
int pixelMask(int addr, int F18ASpriteColorLine[]);

#endif