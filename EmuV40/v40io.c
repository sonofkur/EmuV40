#include <stdint.h>
#include <string.h>
#include "v40io.h"
#include "ports.h"
#include "i8259.h"
//#include "i8253.h"
#include "pit.h"
//#include "i8237.h"

struct structv40io v40;

static uint8_t inv40io(uint16_t portnum) {
	switch (portnum & 0x0F) {
	case 0x00:
		return v40.tcks;
	case 0x02:
		return v40.rfc;
	case 0x04:
		return v40.wmb;
	case 0x05:
		return v40.wcy1;
	case 0x06:
		return v40.wcy2;
	case 0x08:
		return v40.sula;
	case 0x09:
		return v40.tulal;
	case 0x0A:
		return v40.iula;
	case 0x0B:
		return v40.dula;
	case 0x0C:
		return v40.opha;
	case 0x0D:
		return v40.opsel;
	case 0x0E:
		return v40.opcn;
	}

	return 0;
}

static void outv40io(uint16_t portnum, uint8_t value) {

	uint16_t beginAddr;
	uint16_t endAddr;

	switch (portnum & 0x0F) {
	case 0x00:
		v40.tcks = value;
		break;
	case 0x02:
		v40.rfc = value;
		break;
	case 0x04:
		v40.wmb = value;
		break;
	case 0x05:
		v40.wcy1 = value;
		break;
	case 0x06:
		v40.wcy2 = value;
		break;
	case 0x08:
		v40.sula = value;
		beginAddr = ((v40.opha << 8) | (v40.sula));
		endAddr = beginAddr + 1;
		break;
	case 0x09:
		v40.tulal = value;
		beginAddr = ((v40.opha << 8) | (v40.tulal));
		endAddr = beginAddr + 3;
		set_port_write_redirector(beginAddr, endAddr, &outPIT);
		set_port_read_redirector(beginAddr, endAddr, &inPIT);
		break;
	case 0x0A:
		v40.iula = value;
		beginAddr = ((v40.opha << 8) | (v40.iula));
		endAddr = beginAddr + 1;
		set_port_write_redirector(beginAddr, endAddr, &out8259);
		set_port_read_redirector(beginAddr, endAddr, &in8259);
		break;
	case 0x0B:
		v40.dula = value;
		beginAddr = ((v40.opha << 8) | (v40.dula));
		endAddr = beginAddr + 0x0F;
		/*set_port_write_redirector(beginAddr, endAddr, &out8237);
		set_port_read_redirector(beginAddr, endAddr, &in8237);*/
		break;
	case 0x0C:
		v40.opha = value;
		break;
	case 0x0D:
		v40.opsel = value;
		//i8237.enabled = (((v40.opsel >> 0) & 1); //DMA
		//i8259.enabled = ((v40.opsel >> 1) & 1); //INT
		//i8253.enabled = ((v40.opsel >> 2) & 1); //Timer
		//i8251.enabled = (((v40.opsel >> 3) & 1); //UART

		break;
	case 0x0E:
		v40.opcn = value;
		break;
	}
}

void initv40io(void) {
	memset((void*)&v40, 0, sizeof(v40));
	set_port_write_redirector(0xFFE0, 0xFFFE, &outv40io);
	set_port_read_redirector(0xFFE0, 0xFFFE, &inv40io);
}