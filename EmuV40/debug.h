#ifndef DEBUG_H
#define DEBUG_H

#include <string.h>
#include <SDL.h>

#define CHAR_SCALE 		(3)	 // character pixel size.

#define DBG_WIDTH 		(60)									// Char cells across
#define DBG_HEIGHT 		(60)

#define DBG_ASMX 		(1)										// Disassembly starts here
#define DBG_LBLX 		(1) 									// Debug labels start here
#define DBG_DATX		(4)									// Debug data starts here.
#define DBG_STCK		(20)									// Debug stack starts here.
#define DBG_MEMX 		(1)										// Memory Display starts here
#define DBG_ZP_REG		(45)									// Zero page registers start here

#define DMODE_STOP 		(0)										// Debugger is waiting for action.
#define DMODE_STEP 		(1)										// Debugger is doing a single step
#define DMODE_RUN 		(2)										// Debugger is running normally.


struct breakpoint {
	uint16_t ip;
	uint16_t cs;
};

extern int xPos;
extern int yPos;
extern int showDebugOnRender;

void db_init_chars(SDL_Renderer* renderer);
void db_write(SDL_Renderer* renderer, int x, int y, int ch, SDL_Color colour);
void db_string(SDL_Renderer* renderer, int x, int y, char* s, SDL_Color colour);
char* ltrim(char* s);

void db_render_display(int width, int height);
void db_break_to_debugger(void);
int db_get_current_status(void);
void db_set_breakpoint(struct breakpoint newBreakPoint);
void db_init_ui(SDL_Renderer* pRenderer);
void db_free_ui(void);

void db_set_current_mode(int mode);

#endif