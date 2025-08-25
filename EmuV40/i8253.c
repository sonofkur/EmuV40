#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <memory.h>

#include "i8253.h"

#include "mutex.h"
#include "ports.h"
#include "timing.h"


struct i8253_s i8253;


void out8253(uint16_t portnum, uint8_t value)
{
	uint8_t curbyte = 0; // make gcc happy to have initialized variable :)
	portnum &= 3;
	switch (portnum) {
	case 0:
	case 1:
	case 2: //channel data
		if ((i8253.accessmode[portnum] == PIT_MODE_LOBYTE) || ((i8253.accessmode[portnum] == PIT_MODE_TOGGLE) && (i8253.bytetoggle[portnum] == 0)))
			curbyte = 0;
		else if ((i8253.accessmode[portnum] == PIT_MODE_HIBYTE) || ((i8253.accessmode[portnum] == PIT_MODE_TOGGLE) && (i8253.bytetoggle[portnum] == 1)))
			curbyte = 1;
		if (curbyte == 0)  //low byte
			i8253.chandata[portnum] = (i8253.chandata[portnum] & 0xFF00) | value;
		else    //high byte
			i8253.chandata[portnum] = (i8253.chandata[portnum] & 0x00FF) | ((uint16_t)value << 8);
		if (i8253.chandata[portnum] == 0)
			i8253.effectivedata[portnum] = 65536;
		else
			i8253.effectivedata[portnum] = i8253.chandata[portnum];
		i8253.active[portnum] = 1;
		tickgap = (uint64_t)((float)hostfreq / (float)((float)1193182 / (float)i8253.effectivedata[0]));
		if (i8253.accessmode[portnum] == PIT_MODE_TOGGLE)
			i8253.bytetoggle[portnum] = (~i8253.bytetoggle[portnum]) & 1;
		i8253.chanfreq[portnum] = (float)((uint32_t)(((float)1193182.0 / (float)i8253.effectivedata[portnum]) * (float)1000.0)) / (float)1000.0;
		//printf("[DEBUG] PIT channel %u counter changed to %u (%f Hz)\n", portnum, i8253.chandata[portnum], i8253.chanfreq[portnum]);
		break;
	case 3: //mode/command
		i8253.accessmode[value >> 6] = (value >> 4) & 3;
		if (i8253.accessmode[value >> 6] == PIT_MODE_TOGGLE)
			i8253.bytetoggle[value >> 6] = 0;
		break;
	default:
		//UNREACHABLE();
		break;
	}
}


uint8_t in8253(uint16_t portnum)
{
	uint8_t curbyte = 0;
	portnum &= 3;
	switch (portnum) {
	case 0:
	case 1:
	case 2: //channel data
		if ((i8253.accessmode[portnum] == 0) || (i8253.accessmode[portnum] == PIT_MODE_LOBYTE) || ((i8253.accessmode[portnum] == PIT_MODE_TOGGLE) && (i8253.bytetoggle[portnum] == 0)))
			curbyte = 0;
		else if ((i8253.accessmode[portnum] == PIT_MODE_HIBYTE) || ((i8253.accessmode[portnum] == PIT_MODE_TOGGLE) && (i8253.bytetoggle[portnum] == 1)))
			curbyte = 1;
		if ((i8253.accessmode[portnum] == 0) || (i8253.accessmode[portnum] == PIT_MODE_LOBYTE) || ((i8253.accessmode[portnum] == PIT_MODE_TOGGLE) && (i8253.bytetoggle[portnum] == 0)))
			curbyte = 0;
		else if ((i8253.accessmode[portnum] == PIT_MODE_HIBYTE) || ((i8253.accessmode[portnum] == PIT_MODE_TOGGLE) && (i8253.bytetoggle[portnum] == 1)))
			curbyte = 1;
		if ((i8253.accessmode[portnum] == 0) || (i8253.accessmode[portnum] == PIT_MODE_TOGGLE))
			i8253.bytetoggle[portnum] = (~i8253.bytetoggle[portnum]) & 1;
		if (curbyte == 0) {	// low byte
			return (uint8_t)i8253.counter[portnum];
		}
		else {		// high byte
			return (uint8_t)(i8253.counter[portnum] >> 8);
		}
		break;
	case 3:
		break;
	default:
		//UNREACHABLE();
		break;
	}
	return 0;
}


void init8253(void)
{
	memset(&i8253, 0, sizeof(i8253));
	//set_port_write_redirector(0x40, 0x43, &out8253);
	//set_port_read_redirector(0x40, 0x43, &in8253);
}