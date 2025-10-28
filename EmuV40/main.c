#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <SDL.h>
#include <stdbool.h>
#include <Windows.h>
#include "machine.h"
#include "parsecl.h"
#include "ports.h"

//#define USE_VERA
//#define USE_TMS9918A
//#define TICKS_FOR_NEXT_FRAME (1000 / 60)
//
//uint8_t running = 1;
//uint8_t MHZ = 8;
//bool log_video = false;
//bool enable_midline = false;
//bool warp_mode = false;
//bool grab_mouse = false;
//bool disable_emu_cmd_keys = false;
//bool reset_requested = false;
//bool debugger_enabled = false;
//bool console_enabled = true;
//bool hard_reset = false;
//bool trace_mode = false;
//
//bool fullscreen = false;
//int window_scale = 1;
//float screen_x_scale = 1.0;
//float window_opacity = 1.0;
//char* scale_quality = "best";
//
//bool new_frame = false;

bool debugger_enabled;
bool console_enabled;
bool trace_mode;
uint8_t running;
uint8_t MHZ;
bool log_video;
bool enable_midline;
bool warp_mode;
bool grab_mouse;
bool disable_emu_cmd_keys;
bool reset_requested;
bool hard_reset;

//uint32_t frame_count = 0;


uint8_t* bios;
static uint64_t starttick, endtick;
//
//bool key_r = false;
//bool key_s = false;
//bool key_l = false;
//
//uint8_t scrollX = 0, scrollY = 0;
//
//int sdl_live;
static double perfFreq;
static double lastTime;
static int tickCount = 0;
static char tempBuffer[256];


MACHINE machine;


static uint32_t load_bios(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL)
		return -1;
	fseek(file, 0L, SEEK_END);
	uint32_t readsize = ftell(file);
	rewind(file);

	bios = (uint8_t*)malloc(sizeof(uint8_t) * readsize);
	if (bios == NULL)
		return -1;
	fread(bios, sizeof(uint8_t) * readsize, 1, file);

	if (readsize <= 0)
		return 0;
	memcpy(machine.cpu.RAM + 0x100000 - readsize, bios, readsize);
	printf("BIOS %s loaded at 0x%05X (%d KB)\n", filename, 0x100000 - readsize, readsize >> 10);
	memset(machine.cpu.readonly + 0x100000 - readsize, 1, readsize);
	fclose(file);
	free(bios);
	return readsize;
}


void emu_init() {

	ports_init();

	machine_init(&machine);
	machine_reset(&machine);

	load_bios("bios.bin");
}


void emu_reset() {
	machine_reset(&machine);
}

static void doTick() {

	lastTime = (double)SDL_GetPerformanceCounter() / perfFreq;

	double deltaTime = 0.0001;
	uint32_t deltaClockTicks = (uint32_t)(CPU_SPEED * deltaTime);

	double currentTime = (double)SDL_GetPerformanceCounter() / perfFreq;

	if (currentTime - lastTime > 0.1) {
		lastTime = ((double)SDL_GetPerformanceCounter() / perfFreq) + deltaTime;
	}
	else {
		lastTime += deltaTime;
	}

	while ((currentTime = (double)SDL_GetPerformanceCounter() / perfFreq) < lastTime);


	/*for (size_t i = 0; i < deviceCount; ++i) {
		tickDevice(&devices[i], deltaClockTicks, deltaTime);
	}*/
}

static void doRender() {

}

static void doEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			if (event.type == SDL_QUIT) {
				running = false;
				return;
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = false;
				return;
			}
			else {
				case SDL_WINDOWEVENT:
					switch (event.window.event) {
						case SDL_WINDOWEVENT_CLOSE:
							running = false;
							break;

						default:
							break;
					}
				break;
			}
		}
	}
}

void emu_loop() {
	static uint32_t lastRenderTicks = 0;
	
	doTick();

	++tickCount;

	uint32_t currentTicks = SDL_GetTicks();
	if ((currentTicks - lastRenderTicks) > 17) {
		doRender();

		lastRenderTicks = currentTicks;
		tickCount = 0;

		doEvents();

		SDL_snprintf(tempBuffer, sizeof(tempBuffer), "V40Emu (CPU: %0.4f%%) (ROM: %s)", 0.0f, /*getCpuUtilization(cpuDevice) * 100.0f*/ "bios.bin");
		//SDL_SetWindowTitle(tmsVDP.window, tempBuffer);
	}
}

void emu_destroy() {

}

int main(int argc, char** argv) {

	parsecl(argc, argv);

	emu_init();

	cpu_int_check(&machine.cpu, &machine.i8259);
	cpu_exec(&machine.cpu, 100000);

	/*while (machine.running) {
		cpu_int_check(&machine.cpu, &machine.i8259);
		cpu_exec(&machine.cpu, 100000);
	}*/

	/*while (running) {
		emu_loop();
	}*/

	endtick = (SDL_GetTicks() - starttick) / 1000;
	if (endtick == 0)
		endtick = 1; //avoid divide-by-zero exception in the code below, if ran for less than 1 second

	/*printf("\n%lu instructions executed in %lu seconds.\n", (long unsigned int)cpu.totalexec, (long unsigned int)endtick);
	printf("Average speed: %lu instructions/second.\n", (long unsigned int)(cpu.totalexec / endtick));*/

	//printf("NMI count: %d\n", ppu.nmi_count);
	//printf("Frame count: %d\n", frame_count);
	//printf("Average FPS: %d\n", (long unsigned int)(frame_count / endtick));

	emu_destroy();

	return 0;
}
