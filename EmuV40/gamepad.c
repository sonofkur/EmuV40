#include "gamepad.h"
#include "kb.h"
#include <stdio.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

#define   SET_BIT(n,b)    (n |= (1<<b))
#define   CLEAR_BIT(n,b)  (n &= ~(1<<b))

GAMEPAD gp1;
GAMEPAD gp2;

uint8_t gamepad_1_data = 0xFF;

void gamepad_init(GAMEPAD* gp) {
	gamepad_reset(gp);
}

void gamepad_reset(GAMEPAD* gp) {
	gp->latch = 0;
	gp->prev_latch = 0;
	gp->clock = 0;
	gp->prev_clock = 0;
	gp->data = 0;
	gp->is_latched = 0;
	gp->is_clocked = 0;
	gp->curr_shift_bit = 0;
}

void gamepad_latch(GAMEPAD* gp, uint8_t value) {
	gp->latch = value;
	if (gp->latch != gp->prev_latch) {
		if (gp->prev_latch == 1 && gp->latch == 0) {
			gp->is_latched = 1;
			gp->curr_shift_bit = 0;
			gamepad_get_data(gp); // A button is available
		}
		gp->prev_latch = gp->latch;
	}
}

void gamepad_clock(GAMEPAD* gp, uint8_t value) {
	gp->clock = value;
	if (gp->clock != gp->prev_clock) {
		if (gp->prev_clock == 1 && gp->clock == 0) {
			gp->is_clocked = 1;
			gp->curr_shift_bit++;
			gamepad_get_data(gp);
		}
		gp->prev_clock = gp->clock;
	}
}

void gamepad_get_data(GAMEPAD* gp) {
	uint8_t old = gamepad_1_data;
	switch (gp->curr_shift_bit) {
	case 0:
		gp->data = keys['n'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 0); else CLEAR_BIT(gamepad_1_data, 0);
		break;
	case 1:
		gp->data = keys['m'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 1); else CLEAR_BIT(gamepad_1_data, 1);
		break;
	case 2:
		gp->data = keys['j'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 2); else CLEAR_BIT(gamepad_1_data, 2);
		break;
	case 3:
		gp->data = keys['k'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 3); else CLEAR_BIT(gamepad_1_data, 3);
		break;
	case 4:
		gp->data = keys['w'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 4); else CLEAR_BIT(gamepad_1_data, 4);
		break;
	case 5:
		gp->data = keys['s'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 5); else CLEAR_BIT(gamepad_1_data, 5);
		break;
	case 6:
		gp->data = keys['a'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 6); else CLEAR_BIT(gamepad_1_data, 6);
		break;
	case 7:
		gp->data = keys['d'] ? 0 : 1;
		if (gp->data) SET_BIT(gamepad_1_data, 7); else CLEAR_BIT(gamepad_1_data, 7);
		gp->is_latched = 0;
		gp->is_clocked = 0;
		break;
	}

	if (old != gamepad_1_data) {
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gamepad_1_data));
		printf("\n");
	}
		
}