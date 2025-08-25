#include <stdint.h>
#include <stdio.h>
#include "ports.h"
#include "cpu.h"

//#define DEBUG_PORT_TRAFFIC

uint8_t portram[0x10000];


static io_write8_cb_t  port_write_callback[0x10000];
static io_read8_cb_t   port_read_callback[0x10000];
static io_write16_cb_t port_write_callback16[0x10000];
static io_read16_cb_t  port_read_callback16[0x10000];

static void unknown_port_writer(uint16_t portnum, uint8_t value)
{
	printf("IO: unknown port OUT to %04Xh with value %02Xh\n", portnum, value);
}

static uint8_t unknown_port_reader(uint16_t portnum)
{
	printf("IO: unknown port IN to %04Xh\n", portnum);
	return 0xFF;
}

static void sliced_port16_writer(uint16_t portnum, uint16_t value)
{
#ifdef DEBUG_PORT_TRAFFIC
	printf("IO: writing WORD port %Xh with data %04Xh\n", portnum, value);
#endif
	portout(portnum, (uint8_t)value);
	portout(portnum + 1, (uint8_t)(value >> 8));
}

static uint16_t sliced_port16_reader(uint16_t portnum)
{
	uint16_t ret = (uint16_t)portin(portnum);
	ret |= ((uint16_t)portin(portnum + 1) << 8);
#ifdef DEBUG_PORT_TRAFFIC
	printf("IO: reading WORD port %Xh with result of data %04Xh\n", portnum, ret);
#endif
	return ret;
}


void ports_init(void)
{
	for (int a = 0; a < 0x10000; a++) {
		port_write_callback[a] = unknown_port_writer;
		port_read_callback[a] = unknown_port_reader;
		port_write_callback16[a] = sliced_port16_writer;
		port_read_callback16[a] = sliced_port16_reader;
	}
}


void portout(uint16_t portnum, uint8_t value)
{
#ifdef DEBUG_PORT_TRAFFIC
	printf("IO: writing BYTE port %Xh with data %02Xh\n", portnum, value);
#endif
	portram[portnum] = value;
	port_write_callback[portnum](portnum, value);
}


uint8_t portin(uint16_t portnum)
{
#ifdef DEBUG_PORT_TRAFFIC
	printf("IO: reading BYTE port %Xh\n", portnum);
#endif
	return port_read_callback[portnum](portnum);
}


void portout16(uint16_t portnum, uint16_t value)
{
	port_write_callback16[portnum](portnum, value);
}


uint16_t portin16(uint16_t portnum)
{
	return port_read_callback16[portnum](portnum);
}


void set_port_write_redirector(uint16_t startport, uint16_t endport, io_write8_cb_t callback)
{
	while (startport <= endport)
		port_write_callback[startport++] = callback;
}


void set_port_read_redirector(uint16_t startport, uint16_t endport, io_read8_cb_t callback)
{
	while (startport <= endport)
		port_read_callback[startport++] = callback;
}