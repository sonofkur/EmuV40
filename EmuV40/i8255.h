#ifndef I8255_H
#define I8255_H

#include <stdint.h>

struct structi8255 {
	uint8_t pa;
	uint8_t pb;
	uint8_t pc;
	uint8_t ctrl;
};

extern struct structi8255 i8255;
extern void init8255(void);
extern void reset8255(void);

extern uint8_t in8255(uint16_t portnum);
extern void out8255(uint16_t portnum, uint8_t value);

#endif