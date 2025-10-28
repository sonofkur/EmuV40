#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>
#include "cpu.h"

#define PORTS_COUNT 0x1000

extern uint8_t(*ports_cbReadB[PORTS_COUNT])(void* udata, uint32_t portnum);
extern uint16_t(*ports_cbReadW[PORTS_COUNT])(void* udata, uint32_t portnum);
extern void (*ports_cbWriteB[PORTS_COUNT])(void* udata, uint32_t portnum, uint8_t value);
extern void (*ports_cbWriteW[PORTS_COUNT])(void* udata, uint32_t portnum, uint16_t value);
extern void* ports_udata[PORTS_COUNT];

void ports_cbRegister(uint32_t start, uint32_t count, uint8_t(*readb)(void*, uint32_t), uint16_t(*readw)(void*, uint32_t), void (*writeb)(void*, uint32_t, uint8_t), void (*writew)(void*, uint32_t, uint16_t), void* udata);
void ports_init();
void port_write(CPU* cpu, uint16_t portnum, uint8_t value);
void port_writew(CPU* cpu, uint16_t portnum, uint16_t value);
uint8_t port_read(CPU* cpu, uint16_t portnum);
uint16_t port_readw(CPU* cpu, uint16_t portnum);

#endif