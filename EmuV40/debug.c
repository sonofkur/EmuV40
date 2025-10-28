#include "debug.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "cpu.h"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7
#define TEXTURE_HEIGHT (CHAR_HEIGHT * 1)
#define TEXTURE_WIDTH (CHAR_WIDTH * 0x60)

int xPos = 0;
int yPos = 0;

SDL_Texture* fontTexture;
int textureInitialized = 0;



//
// 0-9A-F sets the program address, with shift sets the data address.
//
#define DBGKEY_HOME     SDLK_F1                         // F1 is "Goto PC"
#define DBGKEY_RESET    SDLK_F2                         // F2 resets the 6502
#define DBGKEY_RUN      SDLK_F5                         // F5 is run.
#define DBGKEY_SETBRK   SDLK_F9                         // F9 sets breakpoint
#define DBGKEY_STEP     SDLK_F11                        // F11 is step into.
#define DBGKEY_STEPOVER SDLK_F10                        // F10 is step over.
#define DBGKEY_PAGE_NEXT	SDLK_KP_PLUS
#define DBGKEY_PAGE_PREV	SDLK_KP_MINUS

#define DBGSCANKEY_BRK  SDL_SCANCODE_F12                // F12 is break into running code.
#define DBGSCANKEY_SHOW SDL_SCANCODE_TAB                // Show screen key.
														// *** MUST BE SCAN CODES ***

#define DBGMAX_ZERO_PAGE_REGISTERS 20

#define DDUMP_RAM	0
#define DDUMP_PPU	1

#define DB_WINDOW_WIDTH 640
#define DB_WINDOW_HEIGHT 480

enum DBG_CMD { CMD_DUMP_MEM = 'm', CMD_DUMP_VERA = 'v', CMD_DISASM = 'd', CMD_SET_BANK = 'b', CMD_SET_REGISTER = 'r', CMD_FILL_MEMORY = 'f' };

// RGB colours
const SDL_Color col_bkgnd = { 0, 0, 0, 255 };
const SDL_Color col_label = { 0, 255, 0, 255 };
const SDL_Color col_data = { 0, 255, 255, 255 };
const SDL_Color col_highlight = { 255, 255, 0, 255 };
const SDL_Color col_cmdLine = { 255, 255, 255, 255 };

const SDL_Color col_vram_tilemap = { 0, 255, 255, 255 };
const SDL_Color col_vram_tiledata = { 0, 255, 0, 255 };
const SDL_Color col_vram_special = { 255, 92, 92, 255 };
const SDL_Color col_vram_other = { 128, 128, 128, 255 };

int showDebugOnRender = 0;                               // Used to trigger rendering in video.c
int showFullDisplay = 0;                                 // If non-zero show the whole thing.
uint16_t currentIP = -1;                                      // Current IP value.
uint16_t currentCS = -1;
int currentData = 0x0400;                                     // Current data display address.
int currentPCBank = -1;
int currentBank = -1;
int currentMode = DMODE_RUN;                             // Start running.

int dumpmode = DDUMP_RAM;

struct breakpoint breakPoint = { -1, -1 };               // User Break
struct breakpoint stepBreakPoint = { -1, -1 };           // Single step break.

char cmdLine[64] = "";                                    // command line buffer
int currentPosInLine = 0;                                 // cursor position in the buffer (NOT USED _YET_)
int currentLineLen = 0;                                   // command line buffer length

static SDL_Renderer* db_renderer;                            // Renderer passed in.
static SDL_Window* db_window;

static bool db_build_cmd_line(SDL_Keycode key);
static void db_handle_key_event(SDL_Keycode key, int isShift);
static inline bool db_hit_breakpoint(uint16_t ip, uint16_t cs, struct breakpoint bp);
static void db_exec_cmd(void);
static void db_render_cmd_line(int x, int width, int height);

static void db_number(int x, int y, int n, int w, SDL_Color colour);
static void db_address(int x, int y, int bank, int addr, SDL_Color colour);
static void db_vram_address(int x, int y, int addr, SDL_Color colour);

static int db_render_registers(void);
static void db_render_stack(int bytesCount);
static void db_render_code(int lines, int initialPC);
static void db_render_data(int y, int data);

void db_init_ui(SDL_Renderer* pRenderer) {

	if (pRenderer != NULL) {
		db_renderer = pRenderer;				// Save renderer
	}
	else {
		uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
		SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1"); // Grabs keyboard shortcuts from the system during window grab
		SDL_CreateWindowAndRenderer((int)(DB_WINDOW_WIDTH), (int)(DB_WINDOW_HEIGHT), window_flags, &db_window, &db_renderer);
		SDL_RenderSetLogicalSize(db_renderer, DB_WINDOW_WIDTH, DB_WINDOW_HEIGHT);
		SDL_SetWindowPosition(db_window, 900, 200);
	}
	db_init_chars(db_renderer);
}

void db_free_ui(void) {
	SDL_DestroyRenderer(db_renderer);
	SDL_DestroyWindow(db_window);
}

void db_set_current_mode(int mode) {
	currentMode = mode;
}

void db_render_display(int width, int height) {
	SDL_Rect rc;
	rc.w = DBG_WIDTH * 6 * CHAR_SCALE;							// Erase background, set up rect
	rc.h = height;
	//xPos = width - rc.w; 
	xPos = 0;
	yPos = 0; 								// Position screen
	rc.x = xPos;
	rc.y = yPos; 									// Set rectangle and black out.
	SDL_SetRenderDrawColor(db_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(db_renderer, &rc);

	db_render_registers();
	db_render_data(21, currentData);
	db_render_stack(20);

	SDL_RenderPresent(db_renderer);
}

static char* labels[] = { "ODITSZ-A-P-C","","","AX","BX","CX","DX","","SP", "BP","","SI","DI","", "CS","DS","ES","SS", NULL };
static int db_render_registers(void) {
	int n = 0, yc = 0;
	while (labels[n] != NULL) {									// Labels
		db_string(db_renderer, DBG_LBLX, n, labels[n], col_label); n++;
	}
	yc++;
	/*db_number(DBG_LBLX, yc, cpu.of, 1, col_data);
	db_number(DBG_LBLX + 1, yc, cpu.df, 1, col_data);
	db_number(DBG_LBLX + 2, yc, cpu.ifl, 1, col_data);
	db_number(DBG_LBLX + 3, yc, cpu.tf, 1, col_data);
	db_number(DBG_LBLX + 4, yc, cpu.sf, 1, col_data);
	db_number(DBG_LBLX + 5, yc, cpu.zf, 1, col_data);
	db_number(DBG_LBLX + 7, yc, cpu.af, 1, col_data);
	db_number(DBG_LBLX + 9, yc, cpu.pf, 1, col_data);
	db_number(DBG_LBLX + 11, yc, cpu.cf, 1, col_data);
	yc += 2;
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regax], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regbx], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regcx], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regdx], 4, col_data);
	yc++;
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regsp], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regbp], 4, col_data);
	yc++;
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regsi], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.regs.wordregs[regdi], 4, col_data);
	yc++;
	db_number(DBG_DATX, yc++, cpu.segregs[regcs], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.segregs[regds], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.segregs[reges], 4, col_data);
	db_number(DBG_DATX, yc++, cpu.segregs[regss], 4, col_data);*/
	return n;
}

static void db_render_stack(int bytesCount) {

	/*int start = cpu.regs.wordregs[regsp] + 0;
	int end = cpu.regs.wordregs[regsp] - 16;

	int y = 2;
	db_string(db_renderer, DBG_STCK, 0, "STACK", col_label);
	db_string(db_renderer, DBG_STCK, 1, "---------------", col_label);
	for (int i = end; i <= start; i += 1) {
		uint32_t addr = (cpu.segregs[regss] << 4) + i;
		uint8_t data = cpu.RAM[addr & 0xFFFFF];

		db_number(DBG_STCK, y, addr, 4, addr == ((cpu.segregs[regss] << 4) + cpu.regs.wordregs[regsp]) ? col_highlight : col_label);
		db_number(DBG_STCK + 6, y, data, 2, addr == ((cpu.segregs[regss] << 4) + cpu.regs.wordregs[regsp]) ? col_highlight : col_data);

		y++;
	}*/
}

static void db_render_code(int lines, int initialPC) {

}

static void db_render_data(int y, int data) {
	//db_string(db_renderer, DBG_MEMX, 19, "MEMORY", col_label);
	//db_string(db_renderer, DBG_MEMX, 20, "--------00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F", col_label);

	//while (y < DBG_HEIGHT - 2) {

	//	int bank = (data >> 16) & 0xF;

	//	db_address(DBG_MEMX, y, bank, data & 0xFFFF, col_label);	// Show label.

	//	for (int i = 0; i < 16; i++) {
	//		//int byte = cpu.RAM[(data + i) & 0xFFFFF]; // real_read6502((data + i) & 0xFFFF, true, currentBank);
	//		db_number(DBG_MEMX + 8 + i * 3, y, byte, 2, col_data);
	//		//db_write(db_renderer, DBG_MEMX + 33 + i, y, byte, col_data);
	//	}
	//	y++;
	//	data += 16;
	//}
}

static void db_render_cmd_line(int x, int width, int height) {

}

static bool db_build_cmd_line(SDL_Keycode key) {
	return false;
}

static void db_handle_key_event(SDL_Keycode key, int isShift) {
	//uint16_t opcode;

	//switch (key) {
	//case DBGKEY_STEP:
	//	currentMode = DMODE_STEP;
	//	//cpu clock ticks?
	//	break;
	//case DBGKEY_STEPOVER:
	//	opcode = cpu.RAM[(cpu.segregs[regcs] << 4) + cpu.ip];

	//	break;

	//case DBGKEY_RUN:
	//	currentMode = DMODE_RUN;
	//	//debugCPUClocks = clockticks6502;
	//	//timing_init();
	//	break;

	//case DBGKEY_SETBRK:								// F9 Set breakpoint to displayed.
	//	breakPoint.ip = currentIP;
	//	breakPoint.cs = currentCS;
	//	break;

	//case DBGKEY_HOME:								// F1 sets the display PC to the actual one.
	//	currentIP = cpu.ip;
	//	currentCS = cpu.segregs[regcs];
	//	break;
	//	
	//case DBGKEY_RESET:								// F2 reset the 6502
	//	cpu_reset();
	//	currentIP = cpu.ip;
	//	currentCS = cpu.segregs[regcs];
	//	break;

	//case SDLK_PAGEDOWN:
	//	currentData = (currentData + 0x200) & 0xFFFFF;
	//	break;

	//case SDLK_PAGEUP:
	//	currentData = (currentData - 0x200) & 0xFFFFF;
	//	break;

	//case SDLK_DOWN:
	//	currentData = (currentData + 0x10) & 0xFFFFF;
	//	break;

	//case SDLK_UP:
	//	currentData = (currentData - 0x10) & 0xFFFFF;
	//	break;

	//default:
	//	if (db_build_cmd_line(key)) {
	//		printf("cmd line: %s\n", cmdLine);
	//		db_exec_cmd();
	//	}
	//	break;

	//}
}

void db_break_to_debugger(void) {
	/*currentMode = DMODE_STOP;
	currentIP = cpu.ip;
	currentCS = cpu.segregs[regcs];*/
}

// *******************************************************************************************
// Set a new breakpoint address. -1 to disable.
// *******************************************************************************************
void db_set_breakpoint(struct breakpoint newBreakPoint) {
	breakPoint = newBreakPoint;
}

int db_get_current_status(void) {

	//SDL_Event event;
	//if (currentIP < 0) currentIP = cpu.ip;                                      // Initialise current PC displayed.

	//if (currentMode == DMODE_STEP) {                                        // Single step before
	//	//if (currentPC != regs.pc || currentPCBank != getCurrentBank(regs.pc)) {   // Ensure that the PC moved
	//	//	currentPC = regs.pc;                                         // Update current PC
	//	//	currentPCBank = getCurrentBank(regs.pc);                     // Update the bank if we are in upper memory.
	//	//	currentMode = DMODE_STOP;                               // So now stop, as we've done it.
	//	//}
	//}

	//if ((currentMode != DMODE_STOP) && (db_hit_breakpoint(cpu.ip, cpu.segregs[regcs], breakPoint) || db_hit_breakpoint(cpu.ip, cpu.segregs[regcs], stepBreakPoint))) {       // Hit a breakpoint.
	//	//currentPC = regs.pc;                                                         // Update current PC
	//	//currentPCBank = getCurrentBank(regs.pc);                                     // Update the bank if we are in upper memory.
	//	//currentMode = DMODE_STOP;                                               // So now stop, as we've done it.
	//	//stepBreakPoint.pc = -1;                                                 // Clear step breakpoint.
	//	//stepBreakPoint.bank = -1;
	//}

	//if (SDL_GetKeyboardState(NULL)[DBGSCANKEY_BRK]) {                       // Stop on break pressed.
	//	currentMode = DMODE_STOP;
	//	//currentPC = regs.pc;                                                 // Set the PC to what it is.
	//	//currentPCBank = getCurrentBank(regs.pc);                             // Update the bank if we are in upper memory.
	//}

	///*if (currentPCBank < 0 && currentPC >= 0xA000) {
	//	currentPCBank = currentPC < 0xC000 ? memory_get_ram_bank() : memory_get_rom_bank();
	//}*/

	//if (currentMode != DMODE_RUN) {                                         // Not running, we own the keyboard.
	//	//showFullDisplay =                                               // Check showing screen.
	//	//	SDL_GetKeyboardState(NULL)[DBGSCANKEY_SHOW];
	//	while (SDL_PollEvent(&event)) {                                 // We now poll events here.
	//		switch (event.type) {
	//		case SDL_QUIT:                                  // Time for exit
	//			return -1;

	//		case SDL_KEYDOWN:                               // Handle key presses.
	//			db_handle_key_event(event.key.keysym.sym,
	//				SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT));
	//			break;

	//		}
	//	}
	//}

	//showDebugOnRender = (currentMode != DMODE_RUN);                         // Do we draw it - only in RUN mode.
	//if (currentMode == DMODE_STOP) {                                        // We're in charge.
	//	//video_update();
	//	SDL_Delay(10);
	//	return 1;
	//}

	return 0;
}

static inline bool db_hit_breakpoint(uint16_t ip, uint16_t cs, struct breakpoint bp) {
	if (ip == bp.ip && cs == bp.cs) {
		return true;
	}

	return false;
}

static void db_exec_cmd(void) {

}

// *******************************************************************************************
// left trim string
//
char* ltrim(char* s)
{
	while (isspace(*s)) s++;
	return s;
}

// *******************************************************************************************
//
//										Simple 5 x 7 Font
//
// *******************************************************************************************

static unsigned char fontdata[] = {
	0x00, 0x00, 0x00, 0x00, 0x00,   // 0x20 (space)
	0x00, 0x00, 0x5F, 0x00, 0x00,   // 0x21 '!'
	0x00, 0x07, 0x00, 0x07, 0x00,   // 0x22 '"'
	0x14, 0x7F, 0x14, 0x7F, 0x14,   // 0x23 '#'
	0x24, 0x2A, 0x7F, 0x2A, 0x12,   // 0x24 '$'
	0x23, 0x13, 0x08, 0x64, 0x62,   // 0x25 '%'
	0x36, 0x49, 0x55, 0x22, 0x50,   // 0x26 '&'
	0x00, 0x05, 0x03, 0x00, 0x00,   // 0x27 '''
	0x00, 0x1C, 0x22, 0x41, 0x00,   // 0x28 '('
	0x00, 0x41, 0x22, 0x1C, 0x00,   // 0x29 ')'
	0x08, 0x2A, 0x1C, 0x2A, 0x08,   // 0x2A '*'
	0x08, 0x08, 0x3E, 0x08, 0x08,   // 0x2B '+'
	0x00, 0x50, 0x30, 0x00, 0x00,   // 0x2C ','
	0x08, 0x08, 0x08, 0x08, 0x08,   // 0x2D '-'
	0x00, 0x60, 0x60, 0x00, 0x00,   // 0x2E '.'
	0x20, 0x10, 0x08, 0x04, 0x02,   // 0x2F '/'
	0x3E, 0x51, 0x49, 0x45, 0x3E,   // 0x30 '0'
	0x00, 0x42, 0x7F, 0x40, 0x00,   // 0x31 '1'
	0x42, 0x61, 0x51, 0x49, 0x46,   // 0x32 '2'
	0x21, 0x41, 0x45, 0x4B, 0x31,   // 0x33 '3'
	0x18, 0x14, 0x12, 0x7F, 0x10,   // 0x34 '4'
	0x27, 0x45, 0x45, 0x45, 0x39,   // 0x35 '5'
	0x3C, 0x4A, 0x49, 0x49, 0x30,   // 0x36 '6'
	0x01, 0x71, 0x09, 0x05, 0x03,   // 0x37 '7'
	0x36, 0x49, 0x49, 0x49, 0x36,   // 0x38 '8'
	0x06, 0x49, 0x49, 0x29, 0x1E,   // 0x39 '9'
	0x00, 0x36, 0x36, 0x00, 0x00,   // 0x3A ':'
	0x00, 0x56, 0x36, 0x00, 0x00,   // 0x3B ';'
	0x00, 0x08, 0x14, 0x22, 0x41,   // 0x3C '<'
	0x14, 0x14, 0x14, 0x14, 0x14,   // 0x3D '='
	0x41, 0x22, 0x14, 0x08, 0x00,   // 0x3E '>'
	0x02, 0x01, 0x51, 0x09, 0x06,   // 0x3F '?'
	0x32, 0x49, 0x79, 0x41, 0x3E,   // 0x40 '@'
	0x7E, 0x11, 0x11, 0x11, 0x7E,   // 0x41 'A'
	0x7F, 0x49, 0x49, 0x49, 0x36,   // 0x42 'B'
	0x3E, 0x41, 0x41, 0x41, 0x22,   // 0x43 'C'
	0x7F, 0x41, 0x41, 0x22, 0x1C,   // 0x44 'D'
	0x7F, 0x49, 0x49, 0x49, 0x41,   // 0x45 'E'
	0x7F, 0x09, 0x09, 0x01, 0x01,   // 0x46 'F'
	0x3E, 0x41, 0x41, 0x51, 0x32,   // 0x47 'G'
	0x7F, 0x08, 0x08, 0x08, 0x7F,   // 0x48 'H'
	0x00, 0x41, 0x7F, 0x41, 0x00,   // 0x49 'I'
	0x20, 0x40, 0x41, 0x3F, 0x01,   // 0x4A 'J'
	0x7F, 0x08, 0x14, 0x22, 0x41,   // 0x4B 'K'
	0x7F, 0x40, 0x40, 0x40, 0x40,   // 0x4C 'L'
	0x7F, 0x02, 0x04, 0x02, 0x7F,   // 0x4D 'M'
	0x7F, 0x04, 0x08, 0x10, 0x7F,   // 0x4E 'N'
	0x3E, 0x41, 0x41, 0x41, 0x3E,   // 0x4F 'O'
	0x7F, 0x09, 0x09, 0x09, 0x06,   // 0x50 'P'
	0x3E, 0x41, 0x51, 0x21, 0x5E,   // 0x51 'Q'
	0x7F, 0x09, 0x19, 0x29, 0x46,   // 0x52 'R'
	0x46, 0x49, 0x49, 0x49, 0x31,   // 0x53 'S'
	0x01, 0x01, 0x7F, 0x01, 0x01,   // 0x54 'T'
	0x3F, 0x40, 0x40, 0x40, 0x3F,   // 0x55 'U'
	0x1F, 0x20, 0x40, 0x20, 0x1F,   // 0x56 'V'
	0x7F, 0x20, 0x18, 0x20, 0x7F,   // 0x57 'W'
	0x63, 0x14, 0x08, 0x14, 0x63,   // 0x58 'X'
	0x03, 0x04, 0x78, 0x04, 0x03,   // 0x59 'Y'
	0x61, 0x51, 0x49, 0x45, 0x43,   // 0x5A 'Z'
	0x00, 0x00, 0x7F, 0x41, 0x41,   // 0x5B '['
	0x02, 0x04, 0x08, 0x10, 0x20,   // 0x5C '\'
	0x41, 0x41, 0x7F, 0x00, 0x00,   // 0x5D ']'
	0x04, 0x02, 0x01, 0x02, 0x04,   // 0x5E '^'
	0x40, 0x40, 0x40, 0x40, 0x40,   // 0x5F '_'
	0x00, 0x01, 0x02, 0x04, 0x00,   // 0x60 '`'
	0x20, 0x54, 0x54, 0x54, 0x78,   // 0x61 'a'
	0x7F, 0x48, 0x44, 0x44, 0x38,   // 0x62 'b'
	0x38, 0x44, 0x44, 0x44, 0x20,   // 0x63 'c'
	0x38, 0x44, 0x44, 0x48, 0x7F,   // 0x64 'd'
	0x38, 0x54, 0x54, 0x54, 0x18,   // 0x65 'e'
	0x08, 0x7E, 0x09, 0x01, 0x02,   // 0x66 'f'
	0x08, 0x14, 0x54, 0x54, 0x3C,   // 0x67 'g'
	0x7F, 0x08, 0x04, 0x04, 0x78,   // 0x68 'h'
	0x00, 0x44, 0x7D, 0x40, 0x00,   // 0x69 'i'
	0x20, 0x40, 0x44, 0x3D, 0x00,   // 0x6A 'j'
	0x00, 0x7F, 0x10, 0x28, 0x44,   // 0x6B 'k'
	0x00, 0x41, 0x7F, 0x40, 0x00,   // 0x6C 'l'
	0x7C, 0x04, 0x18, 0x04, 0x78,   // 0x6D 'm'
	0x7C, 0x08, 0x04, 0x04, 0x78,   // 0x6E 'n'
	0x38, 0x44, 0x44, 0x44, 0x38,   // 0x6F 'o'
	0x7C, 0x14, 0x14, 0x14, 0x08,   // 0x70 'p'
	0x08, 0x14, 0x14, 0x18, 0x7C,   // 0x71 'q'
	0x7C, 0x08, 0x04, 0x04, 0x08,   // 0x72 'r'
	0x48, 0x54, 0x54, 0x54, 0x20,   // 0x73 's'
	0x04, 0x3F, 0x44, 0x40, 0x20,   // 0x74 't'
	0x3C, 0x40, 0x40, 0x20, 0x7C,   // 0x75 'u'
	0x1C, 0x20, 0x40, 0x20, 0x1C,   // 0x76 'v'
	0x3C, 0x40, 0x30, 0x40, 0x3C,   // 0x77 'w'
	0x44, 0x28, 0x10, 0x28, 0x44,   // 0x78 'x'
	0x0C, 0x50, 0x50, 0x50, 0x3C,   // 0x79 'y'
	0x44, 0x64, 0x54, 0x4C, 0x44,   // 0x7A 'z'
	0x00, 0x08, 0x36, 0x41, 0x00,   // 0x7B '{'
	0x00, 0x00, 0x7F, 0x00, 0x00,   // 0x7C '|'
	0x00, 0x41, 0x36, 0x08, 0x00,   // 0x7D '}'
	0x08, 0x08, 0x2A, 0x1C, 0x08,   // 0x7E ->'
	0x08, 0x1C, 0x2A, 0x08, 0x08,   // 0x7F <-'
};

// *******************************************************************************************
//
//										Initialize charset
//
// *******************************************************************************************

void db_init_chars(SDL_Renderer* renderer) {
	uint16_t textureData[TEXTURE_WIDTH * TEXTURE_HEIGHT];
	memset(textureData, 0, sizeof textureData);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	fontTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STATIC, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	for (int ch = 0x00; ch < 0x60; ch++) {
		int rcx = 0;
		for (int x1 = 0; x1 < 5; x1++) {
			int rcy = 0;
			int pixData = fontdata[ch * 5 + x1];
			while (pixData != 0) {
				textureData[ch * CHAR_WIDTH + rcy * TEXTURE_WIDTH + rcx] = (pixData & 1) ? 0xFFFF : 0x0000;
				pixData = pixData >> 1;
				rcy++;
			}
			rcx++;
		}
	}
	SDL_UpdateTexture(fontTexture, NULL, &textureData, TEXTURE_WIDTH * 2);
	textureInitialized = 1;
}

// *******************************************************************************************
//
//										Write character
//
// *******************************************************************************************

void db_write(SDL_Renderer* renderer, int x, int y, int ch, SDL_Color colour) {
	if (!textureInitialized) {
		db_init_chars(renderer);
	}
	SDL_SetTextureColorMod(fontTexture, colour.r, colour.g, colour.b);
	ch -= 0x20;
	SDL_Rect srcRect = {
		ch * CHAR_WIDTH,
		0,
		CHAR_WIDTH,
		CHAR_HEIGHT
	};
	SDL_Rect dstRect = {
		x * (CHAR_WIDTH + 1) + xPos,
		y * (CHAR_HEIGHT + 1) + yPos,
		CHAR_WIDTH,
		CHAR_HEIGHT
	};
	SDL_RenderCopy(renderer, fontTexture, &srcRect, &dstRect);
}

// *******************************************************************************************
//
//										Write String
//
// *******************************************************************************************

void db_string(SDL_Renderer* renderer, int x, int y, char* s, SDL_Color colour) {
	while (*s != '\0') {
		db_write(renderer, x++, y, *s++, colour);
	}
}

// *******************************************************************************************
//
//									Write Hex Constant
//
// *******************************************************************************************
void db_number(int x, int y, int n, int w, SDL_Color colour) {
	char fmtString[8], buffer[16];
	snprintf(fmtString, sizeof(fmtString), "%%0%dX", w);
	snprintf(buffer, sizeof(buffer), fmtString, n);
	db_string(db_renderer, x, y, buffer, colour);
}

void db_address(int x, int y, int bank, int addr, SDL_Color colour) {
	//char buffer[4];

	/*if (addr >= 0xA000) {
		snprintf(buffer, sizeof(buffer), "%.2X:", bank);
	}
	else {
		strcpy(buffer, "--:");
	}*/

	db_number(x, y, bank, 2, colour);
	db_string(db_renderer, x + 2, y, ":", colour);
	db_number(x + 3, y, addr, 4, colour);
}

void db_vram_address(int x, int y, int addr, SDL_Color colour) {
	db_number(x, y, addr, 5, colour);
}
