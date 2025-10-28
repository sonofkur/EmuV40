//#include "config.h"
//#include <stdio.h>
//#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <memory.h>
//#include <SDL.h>
//#include <stdbool.h>
//#include <Windows.h>
//
//#include "parsecl.h"
//#include "types.h"
//#include "cpu.h"
//#include "ports.h"
//#include "vera.h"
//#include "ppu.h"
//#include "glue.h"
//#include "debug.h"
//#include "v40io.h"
//#include "joystick.h"
//#include "i8255.h"
////#include "i8253.h"
//#include "pit.h"
//#include "gamepad.h"
//#include "console.h"
//#include "timing.h"
//#include "ay8910.h"
//#include "tms9918a.h"
//
//#include "f18a.h"
//
////#define USE_VERA
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
//uint32_t frame_count2 = 0;
//uint8_t* bios;
//static uint64_t starttick, endtick;
//
//bool key_r = false;
//bool key_s = false;
//bool key_l = false;
//
//uint8_t scrollX = 0, scrollY = 0;
//
//int sdl_live;
//static double perfFreq;
//static double lastTime;
//static int tickCount = 0;
//static char tempBuffer[256];
//
//
//
//static uint32_t load_bios(const char* filename) {
//	FILE* file = fopen(filename, "rb");
//	if (file == NULL)
//		return -1;
//	fseek(file, 0L, SEEK_END);
//	uint32_t readsize = ftell(file);
//	rewind(file);
//
//	bios = (uint8_t*)malloc(sizeof(uint8_t) * readsize);
//	if (bios == NULL)
//		return -1;
//	fread(bios, sizeof(uint8_t) * readsize, 1, file);
//
//	if (readsize <= 0)
//		return 0;
//	memcpy(cpu.RAM + 0x100000 - readsize, bios, readsize);
//	printf("BIOS %s loaded at 0x%05X (%d KB)\n", filename, 0x100000 - readsize, readsize >> 10);
//	memset(cpu.readonly + 0x100000 - readsize, 1, readsize);
//	fclose(file);
//	free(bios);
//	return readsize;
//}
//
//
//void emu_init() {
//
//	cpu_reset();
//	uint32_t size = load_bios("bios.bin");
//	if (size <= 0) {
//		printf("Error loading BIOS");
//	}
//
//	printf("Initializing emulated hardware:\n");
//	ports_init();
//
//	printf("  - V40: ");
//	initv40io();
//	puts("OK");
//
//	printf("  - PPU: ");
//	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_TIMER);
//	sdl_live = 1;
//	initTMS9918A();
//	puts("OK");
//
//	if (debugger_enabled) {
//		printf("  - DEBUGGER: ");
//		db_init_ui(NULL);
//		puts("OK");
//	}
//
//	printf("  - I8255: ");
//	init8255();
//	puts("OK");
//
//	printf("  - I8253: ");
//	//init8253();
//	initPIT();
//	puts("OK");
//
//	printf("  - CONSOLE: ");
//	if (!SDL_CreateThread(console_thread, "V40EmuConsoleThread", NULL)) {
//		fprintf(stderr, "Could not create console thread: %s\n", SDL_GetError());
//		return;
//	}
//	puts("OK");
//
//	starttick = SDL_GetTicks();
//	perfFreq = (double)SDL_GetPerformanceFrequency();
//
//	//TEST
//	//tmsInitialiseText(tmsVDP.vdp);
//	VDPinit();
//}
//
//void emu_tms9918a_tick() {
//
//	lastTime = (double)SDL_GetPerformanceCounter() / perfFreq;
//	double deltaTime = 0.0001;
//	uint32_t deltaClockTicks = (uint32_t)(CPU_CLOCK * deltaTime);
//	double currentTime = (double)SDL_GetPerformanceCounter() / perfFreq;
//
//	if (currentTime - lastTime > 0.1) {
//		lastTime = ((double)SDL_GetPerformanceCounter() / perfFreq) + deltaTime;
//	}
//	else {
//		lastTime += deltaTime;
//	}
//
//	while ((currentTime = (double)SDL_GetPerformanceCounter() / perfFreq) < lastTime);
//
//	//tick devices
//	//tickTMS9918A(deltaClockTicks, (float)deltaTime);
//
//	/*for (size_t i = 0; i < deviceCount; ++i)
//	{
//		tickDevice(&devices[i], deltaClockTicks, deltaTime);
//	}*/
//	//while (SDL_GetTicks() - lastTime < TICKS_FOR_NEXT_FRAME) {
//	//    uint32_t clocks = 0;
//	//    clocks = cpu_exec(1);
//	//}
//
//	///*tms9918a_rasterize(vdp);
//
//	//if (!tms9918a_render(vdp_renderer)) {
//	//    running = false;
//	//}*/
//
//	//if (debugger_enabled) {
//	//    db_render_display(640, 480);
//	//}
//	//frame_count2++;
//	//lastTime = SDL_GetTicks();
//}
//
//void emu_reset() {
//	cpu_reset();
//	VDPreset(true);
//	resetAY8910();
//
//	if (hard_reset) {
//		hard_reset = false;
//
//		uint32_t size = load_bios("bios.bin");
//		if (size <= 0) {
//			printf("Error loading BIOS");
//		}
//	}
//
//	reset_requested = false;
//}
//
//static void doTick() {
//
//	lastTime = (double)SDL_GetPerformanceCounter() / perfFreq;
//
//	double deltaTime = 0.0001;
//	uint32_t deltaClockTicks = (uint32_t)(CPU_SPEED * deltaTime);
//
//	double currentTime = (double)SDL_GetPerformanceCounter() / perfFreq;
//
//	if (currentTime - lastTime > 0.1) {
//		lastTime = ((double)SDL_GetPerformanceCounter() / perfFreq) + deltaTime;
//	}
//	else {
//		lastTime += deltaTime;
//	}
//
//	while ((currentTime = (double)SDL_GetPerformanceCounter() / perfFreq) < lastTime);
//
//
//	/*for (size_t i = 0; i < deviceCount; ++i) {
//		tickDevice(&devices[i], deltaClockTicks, deltaTime);
//	}*/
//}
//
//static void doRender() {
//	//renderTMS9918A();
//}
//
//static void doEvents() {
//	SDL_Event event;
//	while (SDL_PollEvent(&event)) {
//		switch (event.type) {
//			if (event.type == SDL_QUIT) {
//				running = false;
//				return;
//			}
//			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
//				running = false;
//				return;
//			}
//			else {
//		case SDL_WINDOWEVENT:
//			switch (event.window.event) {
//			case SDL_WINDOWEVENT_CLOSE:
//				running = false;
//				break;
//
//			default:
//				break;
//			}
//			break;
//			}
//		}
//	}
//}
//
//void emu_loop() {
//	static uint32_t lastRenderTicks = 0;
//
//	doTick();
//
//	++tickCount;
//
//	uint32_t currentTicks = SDL_GetTicks();
//	if ((currentTicks - lastRenderTicks) > 17) {
//		doRender();
//
//		lastRenderTicks = currentTicks;
//		tickCount = 0;
//
//		doEvents();
//
//		SDL_snprintf(tempBuffer, sizeof(tempBuffer), "V40Emu (CPU: %0.4f%%) (ROM: %s)", 0.0f, /*getCpuUtilization(cpuDevice) * 100.0f*/ "bios.bin");
//		SDL_SetWindowTitle(tmsVDP.window, tempBuffer);
//	}
//}
//
//void emu_destroy() {
//
//	if (debugger_enabled)
//		db_free_ui();
//
//	//destroyTMS9918A();
//	VDPshutdown();
//}
//
////static int emu_thread(void* ptr) {
////	puts("CPU: starting to execute.");
////
////	while (running) {
////		if (reset_requested) {
////			emu_reset();
////		}
////
////		if (debugger_enabled) {
////			int dbgCmd = db_get_current_status();
////			if (dbgCmd > 0) continue;
////			if (dbgCmd < 0) break;
////		}
////
////		uint32_t clocks = 0;
////		bool new_frame = false;
////
////		clocks = cpu_exec(1);
////
////		if (cpu.hltstate) {
////			printf("HALT!!!!!");
////			running = false;
////		}
////
////#ifdef USE_VERA
////		new_frame |= vera_step(MHZ, clocks, false);
////
////		if (new_frame) {
////			if (!vera_update()) {
////				running = false;
////			}
////
////			if (debugger_enabled) {
////				db_render_display(640, 480);
////			}
////
////			frame_count2++;
////		}
////#endif
////
////#ifdef USE_TMS9918A
////
////#endif
////
////		Sleep(1);
////	}
////}
//
//int main(int argc, char** argv) {
//
//	parsecl(argc, argv);
//
//	emu_init();
//
//	while (running) {
//		emu_loop();
//	}
//
//	//emu_loop();
//
//	/*if (!SDL_CreateThread(emu_thread, "V40EmuThread", NULL)) {
//		fprintf(stderr, "Could not create the main emuthread: %s\n", SDL_GetError());
//		return -1;
//	}*/
//
//	/*while (running) {
//		Sleep(1);
//	}*/
//	//emu_loop();
//
//	endtick = (SDL_GetTicks() - starttick) / 1000;
//	if (endtick == 0)
//		endtick = 1; //avoid divide-by-zero exception in the code below, if ran for less than 1 second
//
//	printf("\n%lu instructions executed in %lu seconds.\n", (long unsigned int)cpu.totalexec, (long unsigned int)endtick);
//	printf("Average speed: %lu instructions/second.\n", (long unsigned int)(cpu.totalexec / endtick));
//
//	//printf("NMI count: %d\n", ppu.nmi_count);
//	printf("Frame count: %d\n", frame_count2);
//	printf("Average FPS: %d\n", (long unsigned int)(frame_count2 / endtick));
//
//	emu_destroy();
//
//	return 0;
//}
