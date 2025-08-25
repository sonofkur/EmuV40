#include <stdint.h>
#include <string.h>

#include "i8255.h"

#include "ports.h"
#include "cpu.h"

#include "gamepad.h"

/*
	Control word bits
	MODE SELECT (bit 7: 1)
	| C7 | C6 | C5 | C4 | C3 | C2 | C1 | C0 |
	C0..C2: GROUP B control bits:
		C0: port C (lower) in/out:  0=output, 1=input
		C1: port B in/out:          0=output, 1=input
		C2: mode select:            0=mode0 (basic in/out), 1=mode1 (strobed in/out)
	C3..C6: GROUP A control bits:
		C3: port C (upper) in/out:  0=output, 1=input
		C4: port A in/out:          0=output, 1=input
		C5+C6: mode select:         00=mode0 (basic in/out)
									01=mode1 (strobed in/out)
									1x=mode2 (bi-directional bus)
	C7: 1 for 'mode select'
	INTERRUPT CONTROL (bit 7: 0)
	Interrupt handling is currently not implemented
*/

/* mode select or interrupt control */
#define I8255_CTRL_CONTROL          (1<<7)
#define I8255_CTRL_CONTROL_MODE     (1<<7)
#define I8255_CTRL_CONTROL_BIT      (0)

/* port C (lower) input/output select */
#define I8255_CTRL_CLO              (1<<0)
#define I8255_CTRL_CLO_INPUT        (1<<0)
#define I8255_CTRL_CLO_OUTPUT       (0)

/* port B input/output select */
#define I8255_CTRL_B                (1<<1)
#define I8255_CTRL_B_INPUT          (1<<1)
#define I8255_CTRL_B_OUTPUT         (0)

/* group B mode select */
#define I8255_CTRL_BCLO_MODE        (1<<2)
#define I8255_CTRL_BCLO_MODE_0      (0)
#define I8255_CTRL_BCLO_MODE_1      (1<<2)

/* port C (upper) input/output select */
#define I8255_CTRL_CHI              (1<<3)
#define I8255_CTRL_CHI_INPUT        (1<<3)
#define I8255_CTRL_CHI_OUTPUT       (0)

/* port A input/output select */
#define I8255_CTRL_A                (1<<4)
#define I8255_CTRL_A_INPUT          (1<<4)
#define I8255_CTRL_A_OUTPUT         (0)

/* group A mode select */
#define I8255_CTRL_ACHI_MODE        ((1<<6)|(1<<5))
#define I8255_CTRL_ACHI_MODE_0      (0)
#define I8255_CTRL_ACHI_MODE_1      (1<<5)
/* otherwise mode 2 */

/* set/reset bit (for I8255_CTRL_CONTROL_BIT) */
#define I8255_CTRL_BIT              (1<<0)
#define I8255_CTRL_BIT_SET          (1<<0)
#define I8255_CTRL_BIT_RESET        (0)

#define   SET_BIT(n,b)    (n |= (1<<b))
#define   CLEAR_BIT(n,b)  (n &= ~(1<<b))

struct structi8255 i8255;

void init8255(void) {
	memset((void*)&i8255, 0, sizeof(i8255));
	set_port_write_redirector(0x80, 0x83, &out8255);
	set_port_read_redirector(0x80, 0x83, &in8255);

	reset8255();
}

void reset8255(void) {
	i8255.ctrl = I8255_CTRL_CONTROL_MODE |
		I8255_CTRL_CLO_INPUT |
		I8255_CTRL_CHI_INPUT |
		I8255_CTRL_B_INPUT |
		I8255_CTRL_A_INPUT;
	i8255.pa = 0;
	i8255.pb = 0;
	i8255.pc = 0;
}

uint8_t in8255(uint16_t portnum) {
	uint8_t data = 0xFF;

	switch (portnum & 3) {
	case 0:
		if ((i8255.ctrl & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
			data = i8255.pa;
		}
		/*else {
			data = I8255_GET_PA(pins);
		}*/
		break;
	case 1:
		if ((i8255.ctrl & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
			data = i8255.pb;
		}
		break;
	case 2:

		if (gp1.data == 1)
			SET_BIT(i8255.pc, 4);
		else
			CLEAR_BIT(i8255.pc, 4);

		/*if (gp2.data == 1)
			SET_BIT(i8255.pc, 5);
		else
			CLEAR_BIT(i8255.pc, 5);*/
		
		if ((i8255.ctrl & I8255_CTRL_CHI) == I8255_CTRL_CHI_OUTPUT) {
			data = (data & 0x0F) | (i8255.pc & 0xF0);
		}
		if ((i8255.ctrl & I8255_CTRL_CLO) == I8255_CTRL_CLO_OUTPUT) {
			data = (data & 0xF0) | (i8255.pc & 0x0F);
		}

		
		break;
	case 3:
		break;
	}
	return data;
}

void out8255(uint16_t portnum, uint8_t value) {
	switch (portnum & 3) {
	case 0:
		if ((i8255.ctrl & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
			i8255.pa = value;
		}
		break;
	case 1:
		if ((i8255.ctrl & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
			i8255.pb = value;
		}
		break;
	case 2:
		i8255.pc = value;
		break;
	case 3:
		if ((value & I8255_CTRL_CONTROL) == I8255_CTRL_CONTROL_MODE) {
			/* set port mode */
			i8255.ctrl = value;
			i8255.pa = 0;
			i8255.pb = 0;
			i8255.pc = 0;
		}
		else {
			/* set/clear single bit in port C */
			const uint8_t mask = 1 << ((value >> 1) & 7);
			if ((value & I8255_CTRL_BIT) == I8255_CTRL_BIT_SET) {
				i8255.pc |= mask;
			}
			else {
				i8255.pc &= ~mask;
			}

			/*
				send pc & 0x3 to joystick
				pc.0 = latch
				pc.1 = clock
			*/

			gamepad_latch(&gp1, i8255.pc & 1);
			//gamepad_latch(&gp2, i8255.pc & 1);

			gamepad_clock(&gp1, (i8255.pc >> 1) & 1);
			//gamepad_clock(&gp2, (i8255.pc >> 1) & 1);
		}
		break;
	}
}