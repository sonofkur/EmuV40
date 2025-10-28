/*
  Fake86: A portable, open-source 8086 PC emulator.
  Copyright (C)2010-2012 Mike Chambers
			(C)2020      Gabor Lenart "LGB"

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

// Filenames starting with # -> trying to locate file in various "well-known" directories (see hostfs.c)
// Filenames starting with @ -> locate file in the preferences directory
// Filenames otherwise       -> take filename AS-IS!!
#define DEFAULT_BIOS_FILE 	"#pcxtbios.bin"
//#define DEFAULT_FONT_FILE 	"#asciivga.dat"
#define DEFAULT_FONT_FILE	""
#define DEFAULT_ROMBASIC_FILE	"#rombasic.bin"
#define DEFAULT_VIDEOROM_FILE	"#videorom.bin"
#define DEFAULT_IDEROM_FILE	"#ide_xt.bin"

#define USE_OSD

//be sure to only define ONE of the CPU_* options at any given time, or
//you will likely get some unexpected/bad results!

//#define CPU_8086
//#define CPU_186
#define CPU_V20
//#define CPU_286

#if defined(CPU_8086)
#define CPU_CLEAR_ZF_ON_MUL
#define CPU_ALLOW_POP_CS
#else
#define CPU_ALLOW_ILLEGAL_OP_EXCEPTION
#define CPU_LIMIT_SHIFT_COUNT
#endif

#if defined(CPU_V20)
#define CPU_NO_SALC
#endif

#if defined(CPU_286) || defined(CPU_386)
#define CPU_286_STYLE_PUSH_SP
#else
#define CPU_SET_HIGH_FLAGS
#endif

#define TIMING_INTERVAL 15

#define NETWORKING_OLDCARD //planning to support an NE2000 in the future


#define SAMPLE_RATE		48000
#define SAMPLE_BUFFER	4800

#	define LIKELY(__x__)	(__x__)
#	define UNLIKELY(__x__)	(__x__)
#	define INLINE		inline
#	define UNREACHABLE()	UNREACHABLE_FATAL_ERROR()
#	define	DIRSEP_STR	"\\"
#	define	DIRSEP_CHR	'\\'

#define STR_TITLE "EmuV40"
#define STR_VERSION "0.0.0.1"

#define CPU_SPEED 8000000
#define FRAME_CYCLE_COUNT (CPU_SPEED/60)
#define WINDOW_TITLE "V40 Emulator"
#define MOUSE_GRAB_MSG " (Ctrl+M to end mouse/keyboard capture)"

extern bool debugger_enabled;
extern bool console_enabled;
extern bool trace_mode;
//extern uint8_t running;
extern uint8_t MHZ;
extern bool log_video;
extern bool enable_midline;
extern bool warp_mode;
extern bool grab_mouse;
extern bool disable_emu_cmd_keys;
extern bool reset_requested;
extern bool hard_reset;

//#define DEBUG_DMA
//#define DEBUG_VGA
//#define DEBUG_CGA
//#define DEBUG_PIT
//#define DEBUG_PIC
//#define DEBUG_PPI
//#define DEBUG_UART
//#define DEBUG_TCPMODEM
//#define DEBUG_PCSPEAKER
//#define DEBUG_MEMORY
//#define DEBUG_PORTS
//#define DEBUG_TIMING
//#define DEBUG_OPL2
//#define DEBUG_BLASTER
//#define DEBUG_FDC
//#define DEBUG_NE2000
//#define DEBUG_PCAP

#endif