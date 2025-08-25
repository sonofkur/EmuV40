#ifndef I8259_H
#define I8259_H

#include <stdint.h>

struct structpic {
	uint8_t imr; //mask register
	uint8_t irr; //request register
	uint8_t isr; //service register
	uint8_t icwstep; //used during initialization to keep track of which ICW we're at
	uint8_t icw[5];
	uint8_t intoffset; //interrupt vector offset
	uint8_t priority; //which IRQ has highest priority
	uint8_t autoeoi; //automatic EOI mode
	uint8_t readmode; //remember what to return on read register from OCW3
	uint8_t enabled;
};

extern struct structpic i8259;
extern void init8259(void);
extern uint8_t nextintr(void);
extern void doirq(uint8_t irqnum);

extern uint8_t in8259(uint16_t portnum);
extern void out8259(uint16_t portnum, uint8_t value);

#endif