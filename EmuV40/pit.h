#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include <stdlib.h>

#define PIT_CLOCK_SPEED 1193182
#define CHAN0 pit.chan[0]
#define CHAN1 pit.chan[1]
#define CHAN2 pit.chan[2]
#define CHAN(n) pit.chan[n]

#define RW_STATE_LSB 1
#define RW_STATE_MSB 2
#define RW_STATE_WORD 3
#define RW_STATE_WORD_2 4
#define MODE_INTERRUPT_ON_TERMINAL_COUNT 0
#define MODE_HARDWARE_RETRIGGERABLE_ONE_SHOT 1
#define MODE_RATE_GENERATOR 2
#define MODE_SQUARE_WAVE 3
#define MODE_SOFTWARE_TRIGGERED_STROBE 4
#define MODE_HARDWARE_TRIGGERED_STROBE 5
#define CONTROL_ADDRESS 3

#define STATUS_LATCHED 1
#define COUNTER_LATCHED 2

typedef uint64_t itick_t;
itick_t get_now(void);
extern uint32_t ticks_per_second;


struct pit_channel {
    // <<< BEGIN STRUCT "struct" >>>
    uint32_t count, interim_count; // former is actual count, interim_count is temporary value used while loading
    int flipflop;
    int mode, bcd, gate, rw_mode, rmode, wmode;

    uint8_t status_latch;
    uint8_t whats_latched; // A bitmap of what's latched: bit 0-1: status; bit 2-3: counter
    uint16_t counter_latch;

    itick_t last_load_time, last_irq_time;
    uint32_t period;

    uint32_t pit_last_count;

    int timer_flipflop;

    int timer_running;
    // <<< END STRUCT "struct" >>>
};
struct pit {
    int speaker;
    itick_t last;
    struct pit_channel chan[3];
};

extern struct pit pit;

extern void initPIT(void);

extern uint8_t inPIT(uint16_t portnum);
extern void outPIT(uint16_t portnum, uint8_t value);
void pit_channel_reset(struct pit_channel* this);
void pit_reset(void);
void timer_cb(void);
inline itick_t pit_counter_to_itick(uint32_t c);
inline itick_t pit_itick_to_counter(itick_t i);
int pit_get_out(struct pit_channel* pit);
int pit_get_count(struct pit_channel* pit);
void pit_set_count(struct pit_channel* this, int v);
void pit_channel_latch_counter(struct pit_channel* this);
void set_ticks_per_second(uint32_t value);

#endif