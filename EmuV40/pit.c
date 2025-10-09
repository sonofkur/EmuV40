#include "pit.h"
#include "i8259.h"

//#define REALTIME_TIMING

#ifdef REALTIME_TIMING
#include <sys/time.h>
#endif

struct pit pit;

static itick_t tick_base;

// TODO: Make this configurable
#ifndef REALTIME_TIMING
uint32_t ticks_per_second = 50000000;
#else
uint32_t ticks_per_second = 1000000;
itick_t base;
#endif

void set_ticks_per_second(uint32_t value) {
	ticks_per_second = value;
}


itick_t get_now(void) {
#ifndef REALTIME_TIMING
	return tick_base; // TODO +cpu_get_cycles();
#else
	// XXX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	itick_t hi = (itick_t)tv.tv_sec * (itick_t)1000000 + (itick_t)tv.tv_usec;
	if (!base)
		base = hi;
	return hi - base;
#endif
}

void initPIT(void) {
	pit_reset();
}

inline itick_t pit_counter_to_itick(uint32_t c) {
	double time_scale = (double)ticks_per_second / (double)PIT_CLOCK_SPEED;
	return (itick_t)((double)c * time_scale);
	//return scale_ticks(c, PIT_CLOCK_SPEED, TICKS_PER_SECOND);
}
inline itick_t pit_itick_to_counter(itick_t i) {
	double time_scale = (double)PIT_CLOCK_SPEED / (double)ticks_per_second;
	return (itick_t)((double)i * time_scale);
	//return scale_ticks(i, PIT_CLOCK_SPEED, ticks_per_second);
}
/*
static inline itick_t pit_get_time(void)
{
	//return (get_now() * PIT_CLOCK_SPEED) / TICKS_PER_SECOND; // XXXX: Can overflow
	return pit_itick_to_counter(get_now());
}*/

// Notes on PIT modes:
/*
Mode 0:
One-shot mode. OUT line is set high after count goes from one to zero, and is not set back to low again.

Mode 1:
One-shot mode. OUT line is set high after you set count until count goes from one to zero, and is not set back to high again. Mode 0 & 1 are opposites of one another.

Mode 2:
Repeatable. OUT will be high unless count == 1

Mode 3:
Repeatable. If count is odd, out will be high for (n + 1) / 2 counts. Otherwise, OUT will be high for (n - 1) / 2 counts. Afterwards, it will be low until timer is refilled

Mode 4:
One shot mode. Same thing as Mode 2 except it goes low at count == 0

Mode 5:
Same thing as #4, really.
*/
int pit_get_out(struct pit_channel* pit) {
	// Get cycles elapsed since we reloaded the count register
	uint32_t elapsed = pit_itick_to_counter(get_now() - pit->last_load_time);
	if (pit->count == 0) return 0;
	uint32_t current_counter = elapsed % pit->count; // The current value of the counter
	switch (pit->mode) {
	case 0:
	case 1: // XXX : one shot mode?
		return (pit->count >= current_counter) ^ pit->mode; // They are the opposites of each other
	case 2:
		return current_counter != 1;
	case 3: // XXX: Is this right?
		if (pit->count & 1) // odd
			return current_counter >= ((pit->count + 1) >> 1);
		else // even
			return current_counter < ((pit->count - 1) >> 1);
	case 4:
	case 5:
		return current_counter != 0;
	}
	abort();
}

int pit_get_count(struct pit_channel* pit) {
	itick_t elapsed = get_now() - pit->last_load_time;
	uint32_t diff_in_ticks = (uint32_t)((double)elapsed * (double)PIT_CLOCK_SPEED / (double)ticks_per_second);
	uint32_t current = pit->count - diff_in_ticks;
	if (pit->count == 0)
		return 0; // Avoid divide by zero errors for uninitialized timers.
	//if (current & 0x80000000) {
	current = (current % pit->count); // + pit->count;
	//}
	return current;
}

void pit_set_count(struct pit_channel* this, int v) {
	this->last_irq_time = this->last_load_time = get_now(); //pit_get_time();
	this->count = (!v) << 16 | v; // 0x10000 if v is 0
	this->period = pit_counter_to_itick(this->count);
	this->timer_running = 1;
	this->pit_last_count = pit_get_count(this); // should this be 0?
}
void pit_channel_latch_counter(struct pit_channel* this) {
	if (!(this->whats_latched & COUNTER_LATCHED)) {
		uint16_t ct = pit_get_count(this);
		int mode = this->rw_mode;
		this->whats_latched = (mode << 2) | COUNTER_LATCHED;
		switch (mode) {
		case 1: // lobyte or hibyte only
		case 2:
			this->counter_latch = ct >> ((mode - 1) << 3) & 0xFF;
			break;
		case 3: // flipflop
			this->counter_latch = ct;
			break;
		}
	}
}

void pit_channel_reset(struct pit_channel* this) {
	this->count = 0;
	this->flipflop = this->mode = this->bcd = this->gate = 0;
	this->last_load_time = -1;
}

void pit_reset(void) {
	for (int i = 0; i < 3; i++) {
		pit_channel_reset(pit.chan + i);
		pit.chan[i].gate = i != 2;
	}
	pit.speaker = 0;
}
void timer_cb(void) {
	//pic_lower_irq(0);
	//pic_raise_irq(0);
	doirq(0);
}

// Get the number of ticks, in the future, that the PIT needs to wait.
int pit_next(itick_t now) {
	//UNUSED(now);
	uint32_t count = pit_get_count(&pit.chan[0]), raise_irq = 0;
	if (count > pit.chan[0].pit_last_count) {
		// Count has gone from 0 --> 0xFFFF
		raise_irq = 1;
	}
	if (pit.chan[0].timer_running) {

		int refill_count = pit.chan[0].count;
		if (raise_irq) {
			timer_cb();
			if (pit.chan[0].mode != 2 && pit.chan[0].mode != 3) {
				pit.chan[0].timer_running = 0;
				return -1;
			}
		}
		pit.chan[0].pit_last_count = count;
		return pit_counter_to_itick(refill_count - count);
	}
	return -1;
}

uint8_t inPIT(uint16_t portnum) {
	struct pit_channel* chan = &pit.chan[portnum & 3];
	uint8_t retv = -1;
	if (chan->whats_latched & STATUS_LATCHED) {
		chan->whats_latched &= ~STATUS_LATCHED;
		retv = chan->status_latch;
	}
	else if (chan->whats_latched & COUNTER_LATCHED) {
		int whats_latched_temp = chan->whats_latched >> 2;
		switch (whats_latched_temp) {
		case 1: // lobyte
		case 2: // hibyte
			whats_latched_temp = 0;
			retv = chan->counter_latch; // We already did the shifting before we reached this point
			break;
		case 3:
			whats_latched_temp = (2 << 2) | COUNTER_LATCHED; // turn it into "hibyte", although "lobyte" could work just as well
			retv = chan->counter_latch;
			chan->counter_latch >>= 8; // get hibyte
			break;
		}
		//chan->whats_latched |= whats_latched_temp << 1;
		chan->whats_latched = whats_latched_temp;
	}
	else {
		uint32_t count = pit_get_count(chan);
		switch (chan->rmode) {
		case 0:
			retv = count; // automatic truncation
			break;
		case 1:
			retv = count >> 8;
			break;
		case 2:
		case 3:
			retv = count >> ((chan->rmode & 1) << 3); // Select between lobyte and hibyte depending on the lsb
			chan->rmode ^= 1;
			break;
		}
	}
	//PIT_LOG("readb: port=0x%02x, result=0x%02x\n", a, retv);
	return retv;
}

void outPIT(uint16_t portnum, uint8_t value) {
	int channel = portnum & 3;
	switch (channel) {
		case 3:
		{
			// Not a controller, but a command register
			channel = value >> 6;

			uint8_t opmode = value >> 1 & 7,
				bcd = value & 1,
				access = value >> 4 & 3;
			switch (channel) {
				case 3:
					// Read-Back command
					for (int i = 0; i < 3; i++) {
						if ((opmode >> i) & 1) { // The fields mean different things
							struct pit_channel* chan = &pit.chan[i];
							if (!(access & 2)) // Latch count flag
								pit_channel_latch_counter(chan);
							if (!(access & 1)) { // Latch status flag
								if (!(chan->whats_latched & STATUS_LATCHED)) {
									chan->status_latch = (pit_get_out(chan) << 7) | (chan->rw_mode << 4) | //
										(chan->mode << 1) | //
										chan->bcd;
									chan->whats_latched |= STATUS_LATCHED;
								}
							}
						}
					}
					break;
				case 0:
				case 1:
				case 2:
				{
					struct pit_channel* chan = &pit.chan[channel];
					if (!access) {
						//PIT_LOG("I/O Latched counter %d [ticks: %08x]\n", channel, pit_get_count(chan));
						pit_channel_latch_counter(chan);
					}
					else {
						chan->rw_mode = access;

						chan->wmode = chan->rmode = access - 1; // Internal registers

						chan->mode = opmode;
						switch (chan->mode) {
						case 2:
							if (channel == 0)
								doirq(0);
								//pic_raise_irq(0);
							break;
						}
						chan->bcd = bcd;
						if (bcd) {
							//PIT_LOG("BCD mode not supported\n");
						}
					}
					break;
				}
			}
			break;
		}
		case 0:
		case 1:
		case 2:
		{
			struct pit_channel* chan = &pit.chan[channel];
			switch (chan->wmode) {
			case 0:
				pit_set_count(chan, value);
				break;
			case 1:
				pit_set_count(chan, value << 8);
				break;
			case 2:
				chan->interim_count = value;
				chan->wmode ^= 1;
				break;
			case 3:
				pit_set_count(chan, value << 8 | chan->interim_count);
				chan->wmode ^= 1; // ???
				break;
			}
			break;
		}
	}
}

