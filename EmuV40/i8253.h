#ifndef FAKE86_I8253_H_INCLUDED
#define FAKE86_I8253_H_INCLUDED

#include <stdint.h>

#define PIT_MODE_LATCHCOUNT	0
#define PIT_MODE_LOBYTE		1
#define PIT_MODE_HIBYTE		2
#define PIT_MODE_TOGGLE		3

struct i8253_s {
	uint16_t chandata[3];
	uint8_t accessmode[3];
	uint8_t bytetoggle[3];
	uint32_t effectivedata[3];
	float chanfreq[3];
	uint8_t active[3];
	uint16_t counter[3];
};

extern struct i8253_s i8253;
extern void init8253(void);

extern uint8_t in8253(uint16_t portnum);
extern void out8253(uint16_t portnum, uint8_t value);

#endif