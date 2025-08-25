#ifndef V40IO_H
#define V40IO_H

#include <stdint.h>

/**

REG		|	7	|	6	|	5	|	4	|	3	|	2	|	1	|	0	|
OPCN	|	X	|	X	|	X	|	X	|	IRSW		|		PF		|
OPSEL	|	X	|	X	|	X	|	X	| SCU	| TCU	| ICU	| DMA	|
OPHA	|			A15-A7 (high byte of the IO address space)
DULA	|			A7-A4 DMA (low bits of the IO address space)
IULA	|			A7-A2 INT (low bits of the IO address space)
TULAL	|			A7-A2 TIMER (low bits of the IO address space)
SULA	|			A7-A2 SERIAL (low bits of the IO address space)
WCY2	|	X	|	X	|	X	|	X	|	DMA			|		RFSH	| 00-11 (0-3 wait states)
WCY1	|		IO		|		UMW		|		MMW		|		LMW		| 00-11 (0-3 wait states)
WMB		|	X	|			LMB			|	X	|			UMB			|
RFC		| EN	|	X	|	X	| RTM (refresh timer)					|
TCKS	|	X	|	X	|	X	| CS2	| CS1	| CS0	| prescale count|

*/

struct structv40io {
	uint8_t opcn;	//on-chip perf connection - 0xFFFE
	uint8_t opsel;	//on-chip perf selection register - 0xFFFD
	uint8_t opha;	//op-chip perf relocation registers (upper byte) - 0xFFFC
	uint8_t dula;	//DMA relocation register (lower byte) - 0xFFFB
	uint8_t iula;	//interrupt relocation register (lower byte) - 0xFFFA
	uint8_t tulal;	//timer relocation register (lower byte) - 0xFFF9
	uint8_t sula;	//serial relocation register (lower byte) - 0xFFF8
	uint8_t wcy2;	//wait control register - DMA and refreshes - 0xFFF6
	uint8_t wcy1;	//wait control register - Memory and IO - 0xFFF5
	uint8_t wmb;	//wait state memory boundary - 0xFFF4
	uint8_t rfc;	//refresh control register - 0xFFF2
	uint8_t tcks;	//timer clock selection - 0xFFF0
};

extern struct structv40io v40;
void initv40io(void);

#endif