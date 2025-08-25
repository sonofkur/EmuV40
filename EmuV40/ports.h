#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

typedef void     (*io_write8_cb_t)  (uint16_t portnum, uint8_t value);
typedef uint8_t(*io_read8_cb_t)   (uint16_t portnum);
typedef void     (*io_write16_cb_t) (uint16_t portnum, uint16_t value);
typedef uint16_t(*io_read16_cb_t)  (uint16_t portnum);

uint8_t portram[0x10000];

void set_port_write_redirector(uint16_t startport, uint16_t endport, io_write8_cb_t callback);
void set_port_read_redirector(uint16_t startport, uint16_t endport, io_read8_cb_t callback);


uint16_t portin16(uint16_t portnum);
uint8_t portin(uint16_t portnum);
void portout16(uint16_t portnum, uint16_t value);
void portout(uint16_t portnum, uint8_t value);
void ports_init(void);

#endif