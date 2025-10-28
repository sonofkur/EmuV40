#ifndef I8259_H
#define I8259_H

#include <stdint.h>

typedef struct structpic {
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
}I8259;

//extern struct structpic i8259;
void i8259_init(I8259 *pic);
uint8_t i8259_next_intr(I8259* pic);
void i8259_do_irq(I8259* pic, uint8_t irqnum);

uint8_t i8259_read(I8259* i8259, uint16_t portnum);
void i8259_write(I8259* i8259, uint16_t portnum, uint8_t value);

#endif