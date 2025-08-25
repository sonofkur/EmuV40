#ifndef GLUE_H
#define GLUE_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

#define CPU_SPEED 8000000
#define FRAME_CYCLE_COUNT (CPU_SPEED/60)
#define WINDOW_TITLE "V40 Emulator"
#define MOUSE_GRAB_MSG " (Ctrl+M to end mouse/keyboard capture)"

extern bool debugger_enabled;
extern bool console_enabled;
extern bool trace_mode;
extern uint8_t running;
extern uint8_t MHZ;
extern bool log_video;
extern bool enable_midline;
extern bool warp_mode;
extern bool grab_mouse;
extern bool disable_emu_cmd_keys;
extern bool reset_requested;
extern bool hard_reset;

//extern char window_title[];

#endif