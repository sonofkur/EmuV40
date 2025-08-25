#include "console.h"
#include "config.h"
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "cpu.h"
#include "glue.h"

#define strcmpi _strcmpi
#define console_write(n)	printf("%s",n)
#define console_writeln(n)	puts(n)
#define console_printf 		printf

static const char parameter_separator_chars[] = "\t\n\r ";
static const char console_prompt[] = "V40EMU> ";
#define NEXT_TOKEN()	strtok(NULL, parameter_separator_chars)

static inline void consolehelp(void) {
	console_writeln(
		"Console command summary:\n"
		"  The console is not very robust yet. There are only a few commands:\n\n"
		//"    chdisk drv fn     Attach/remove drive 'drv' (fd0,fd1,hd0,hd1) to image file 'fn' (or - to remove)\n"
		"    reset             Reset machine\n"
		"    regs              Dump registers\n"
		"    dump seg ofs      Show memory dump at seg ofs (ofs is optional). All numbers are in hex\n"
		"    help              This help display.\n"
		"    quit              Immediately abort emulation and quit V40Emu."
	);
}

static void waitforcmd(const char* prompt, char* dst, uint16_t maxlen, const char* ans_on_fail) {
	console_write(prompt);
	uint16_t inputptr;
	inputptr = 0;
	maxlen -= 2;
	dst[0] = 0;
	while (running) {
		if (_kbhit()) {
			uint8_t cc = (uint8_t)_getch();
			switch (cc) {
			case 0:
			case 9:
			case 10:
				break;
			case 8: //backspace
				if (inputptr > 0) {
					printf("%c %c", 8, 8);
					dst[--inputptr] = 0;
				}
				break;
			case 13: //enter
				printf("\n");
				return;
			default:
				if (inputptr < maxlen) {
					dst[inputptr++] = cc;
					dst[inputptr] = 0;
					printf("%c", cc);
				}
				break;
			}
		}
		SDL_Delay(10); //don't waste CPU time while in the polling loop
	}
}

int console_thread(void* ptr) {
	char inputline[1024];
	int last_cmd_dump = 0;

	while (running) {
		static const char dump_cmd_name[] = "dump";
		waitforcmd(console_prompt, inputline, sizeof(inputline), "close");
		const char* cmd = strtok(inputline, parameter_separator_chars);
		if (!cmd) {
			if (last_cmd_dump)
				cmd = dump_cmd_name;
			else
				continue;
		}
		last_cmd_dump = 0;
		if (!strcmpi(cmd, "regs")) {
			console_printf(
				"CS:IP=%04X:%04X SS:SP=%04X:%04X DS=%04X ES=%04X\n"
				"AX=%04X BX=%04X CX=%04X DX=%04X SI=%04X DI=%04X BP=%04X\n",
				CPU_CS, CPU_IP, CPU_SS, CPU_SP, CPU_DS, CPU_ES,
				CPU_AX, CPU_BX, CPU_CX, CPU_DX, CPU_SI, CPU_DI, CPU_BP
			);
		}
		else if (!strcmpi(cmd, "help")) {
			consolehelp();
		}
		else if (!strcmpi(cmd, "quit")) {
			running = 0;
		}
		else if (!strcmpi(cmd, "close")) {
			console_writeln("Closing management console on request (or cannot read from console).");
			break;
		}
	}

	console_writeln("Terminating console thread.");
	//hijacked_input = 0;

	return 0;
}