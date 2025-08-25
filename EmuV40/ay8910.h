#ifndef AY3910_H
#define AY3910_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

#define SOUND_FREQ   22050
#define SOUND_SAMPLE  1024

#define MAX_OUTPUT 0x0fff
//#define MAX_OUTPUT 0x7f

#define STEP3 1
#define STEP2 length
#define STEP  2

/* register id's */
#define AY_AFINE	(0)
#define AY_ACOARSE	(1)
#define AY_BFINE	(2)
#define AY_BCOARSE	(3)
#define AY_CFINE	(4)
#define AY_CCOARSE	(5)
#define AY_NOISEPER	(6)
#define AY_ENABLE	(7)
#define AY_AVOL		(8)
#define AY_BVOL		(9)
#define AY_CVOL		(10)
#define AY_EFINE	(11)
#define AY_ECOARSE	(12)
#define AY_ESHAPE	(13)

#define AY_PORTA	(14)
#define AY_PORTB	(15)

void ay8910_init_sound();
void ay8910_done_sound();
void ay8910_write(int r, int v);

uint8_t inAY8910(uint16_t portnum);
void outAY8910(uint16_t portnum, uint8_t value);
void resetAY8910();
#endif