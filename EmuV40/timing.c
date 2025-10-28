#include "config.h"
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
LARGE_INTEGER queryperf;


#include "timing.h"

#include "i8253.h"
#include "i8259.h"
//#include "blaster.h"
//#include "adlib.h"
//#include "audio.h"
//#include "video.h"
//#include "sndsource.h"
//#include "parsecl.h"

uint64_t lasttick;

uint64_t hostfreq = 1000000, tickgap;
static uint64_t curtick = 0, lastscanlinetick, curscanline = 0, i8253tickgap, lasti8253tick, scanlinetiming;
uint64_t sampleticks, gensamplerate;
static uint64_t lastsampletick, ssourceticks, lastssourcetick, adlibticks, lastadlibtick, lastblastertick;

static uint16_t pit0counter = 65535;


void inittiming(void)
{
	QueryPerformanceFrequency(&queryperf);
	hostfreq = queryperf.QuadPart;
	QueryPerformanceCounter(&queryperf);
	curtick = queryperf.QuadPart;
	lasti8253tick = lastblastertick = lastadlibtick = lastssourcetick = lastsampletick = lastscanlinetick = lasttick = curtick;
	scanlinetiming = hostfreq / 31500;
	ssourceticks = hostfreq / 8000;
	adlibticks = hostfreq / 48000;
	/*if (doaudio)
		sampleticks = hostfreq / gensamplerate;
	else*/
	sampleticks = -1;
	i8253tickgap = hostfreq / 119318;
}



void timing(void)
{
	QueryPerformanceCounter(&queryperf);
	curtick = queryperf.QuadPart;

	/*if (curtick >= (lastscanlinetick + scanlinetiming)) {
		curscanline = (curscanline + 1) % 525;
		if (curscanline > 479)
			port3da = 8;
		else
			port3da = 0;
		if (curscanline & 1)
			port3da |= 1;
		pit0counter++;
		lastscanlinetick = curtick;
	}*/
	if (i8253.active[0]) { //timer interrupt channel on i8253
		if (curtick >= (lasttick + tickgap)) {
			lasttick = curtick;
			//i8259_do_irq(0);
		}
	}
	if (curtick >= (lasti8253tick + i8253tickgap)) {
		for (int i8253chan = 0; i8253chan < 3; i8253chan++) {
			if (i8253.active[i8253chan]) {
				if (i8253.counter[i8253chan] < 10) i8253.counter[i8253chan] = i8253.chandata[i8253chan];
				i8253.counter[i8253chan] -= 10;
			}
		}
		lasti8253tick = curtick;
	}
	if (curtick >= (lastssourcetick + ssourceticks)) {
		//tickssource();
		lastssourcetick = curtick - (curtick - (lastssourcetick + ssourceticks));
	}
	/*if (blaster.samplerate > 0) {
		if (curtick >= (lastblastertick + blaster.sampleticks)) {
			tickBlaster();
			lastblastertick = curtick - (curtick - (lastblastertick + blaster.sampleticks));
		}
	}*/
	if (curtick >= (lastsampletick + sampleticks)) {
		//tickaudio();
		/*if (slowsystem) {
			tickaudio();
			tickaudio();
			tickaudio();
		}*/
		lastsampletick = curtick - (curtick - (lastsampletick + sampleticks));
	}
	if (curtick >= (lastadlibtick + adlibticks)) {
		//tickadlib();
		lastadlibtick = curtick - (curtick - (lastadlibtick + adlibticks));
	}
}