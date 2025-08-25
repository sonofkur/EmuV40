#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <stdint.h>

typedef struct structgp {
	uint8_t latch;
	uint8_t prev_latch;
	uint8_t clock;
	uint8_t prev_clock;
	uint8_t data;
	uint8_t is_latched;
	uint8_t is_clocked;
	uint8_t curr_shift_bit;
}GAMEPAD;

extern GAMEPAD gp1;
extern GAMEPAD gp2;

void gamepad_init(GAMEPAD *gp);
void gamepad_reset(GAMEPAD* gp);
void gamepad_latch(GAMEPAD* gp, uint8_t value);
void gamepad_clock(GAMEPAD* gp, uint8_t value);
void gamepad_get_data(GAMEPAD* gp);

#endif