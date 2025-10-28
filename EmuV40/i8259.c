#include <stdint.h>
#include <string.h>

#include "i8259.h"

#include "ports.h"
#include "cpu.h"

uint8_t i8259_read(I8259* i8259, uint16_t portnum) {
	switch (portnum & 1) {
	case 0:
		if (i8259->readmode == 0)
			return i8259->irr;
		else
			return i8259->isr;
	case 1: //read mask register
		return i8259->imr;
	}
	return 0;
}

void i8259_write(I8259* i8259, uint16_t portnum, uint8_t value) {
	//uint8_t i;
	//switch (portnum & 1) {
	//case 0:
	//	if (value & 0x10) { //begin initialization sequence
	//		i8259.icwstep = 1;
	//		i8259.imr = 0; //clear interrupt mask register
	//		i8259.icw[i8259.icwstep++] = value;
	//		return;
	//	}
	//	if ((value & 0x98) == 8) { //it's an OCW3
	//		if (value & 2) i8259.readmode = value & 2;
	//	}
	//	if (value & 0x20) { //EOI command
	//		for (i = 0; i < 8; i++)
	//			if ((i8259.isr >> i) & 1) {
	//				i8259.isr ^= (1 << i);
	//				//if ((i == 0) && (makeupticks > 0)) { makeupticks = 0; i8259.irr |= 1; }
	//				return;
	//			}
	//	}
	//	break;
	//case 1:
	//	if ((i8259.icwstep == 3) && (i8259.icw[1] & 2))
	//		i8259.icwstep = 4; //single mode, so don't read ICW3
	//	if (i8259.icwstep < 5) {
	//		i8259.icw[i8259.icwstep++] = value;
	//		return;
	//	}
	//	//if we get to this point, this is just a new IMR value
	//	i8259.imr = value;
	//	break;
	//}
}

uint8_t i8259_next_intr(I8259* pic) {
	uint8_t i, tmpirr;
	tmpirr = pic->irr & (~pic->imr); //XOR request register with inverted mask register
	for (i = 0; i < 8; i++)
		if ((tmpirr >> i) & 1) {
			pic->irr ^= (1 << i);
			pic->isr |= (1 << i);
			return pic->icw[2] + i;
		}
	return 0; //this won't be reached, but without it the compiler gives a warning
}

void i8259_do_irq(I8259 *pic, uint8_t irqnum) {
	pic->irr |= (1 << irqnum);
}

void i8259_init(I8259* pic) {
	memset((void*)&pic, 0, sizeof(I8259));

	ports_cbRegister(0x20, 2, (void*)i8259_read, NULL, (void*)i8259_write, NULL, pic);
	// set in v40io.c
	//set_port_write_redirector(0x20, 0x21, &out8259);
	//set_port_read_redirector(0x20, 0x21, &in8259);
}