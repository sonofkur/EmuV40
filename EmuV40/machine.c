#include "machine.h"
#include <stdint.h>
#include <string.h>

void machine_init(MACHINE* machine) {
	//memset((void*)&machine, 0, sizeof(MACHINE));
	memset((void*)&machine->cpu, 0, sizeof(CPU));
	cpu_reset(&machine->cpu);

	machine->running = 1;
}

void machine_reset(MACHINE* machine) {
	cpu_reset(&machine->cpu);
}