#ifndef MACHINE_H
#define MACHINE_H

#include "cpu.h"
#include "i8259.h"

typedef struct {
	CPU cpu;
	I8259 i8259;

	uint8_t running;
}MACHINE;

void machine_init(MACHINE* machine);
void machine_reset(MACHINE* machine);


#endif