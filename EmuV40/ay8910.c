#include "ay8910.h"

struct AY8910 {
	int32_t index;
	int32_t ready;
	uint8_t regs[16];
	int32_t lastEnable;
	int32_t periodA, periodB, periodC, periodN, periodE;
	int32_t countA, countB, countC, countN, countE;
	uint32_t volA, volB, volC, volE;
	uint8_t envelopeA, envelopeB, envelopeC;
	uint8_t outputA, outputB, outputC, outputN;
	int8_t countEnv;
	uint8_t hold, alternate, attack, holding;
	int32_t RNG;
	uint8_t latchAddr;
	unsigned int volTable[32];
} PSG;

const int fm_midi_notes[] = {//MIDI note number
  15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204,//0-o7
  9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428,//8-15
  6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050,//16-23
  3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551,//24-31
  2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607,//32-39
  1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012,//40-47
  956, 902, 851, 804, 758, 716, 676, 638,//48-55
  602, 568, 536, 506, 478, 451, 426, 402,//56-63
  379, 358, 338, 319, 301, 284, 268, 253,//64-71
  239, 225, 213, 201, 190, 179, 169, 159,//72-79
  150, 142, 134, 127, 119, 113, 106, 100,//80-87
  95, 89, 84, 80, 75, 71, 67, 63,//88-95
  60, 56, 53, 50, 47, 45, 42, 40,//96-103
  38, 36, 34, 32, 30, 28, 27, 25,//104-111
  24, 22, 21, 20, 19, 18, 17, 16,//112-119
  15, 14, 13, 13, 12, 11, 11, 10,//120-127
  0//off
};

static void ay8910_build_mixer_table();

uint8_t inAY8910(uint16_t portnum) {
	uint8_t data = 0x00;
	switch (portnum & 1) {
	case 0:
		//read
		data = PSG.regs[PSG.latchAddr];
		break;
	case 1:
		break;
	}
	return data;
}

void outAY8910(uint16_t portnum, uint8_t value) {
	switch (portnum & 1) {
	case 0:
		//write
		ay8910_write(PSG.latchAddr, value);
		break;
	case 1:
		// latch
		PSG.latchAddr = (value & 0xF);
		break;
	}
}

void resetAY8910() {
	PSG.regs[0] = 0;
	PSG.regs[1] = 0;
	PSG.regs[2] = 0;
	PSG.regs[3] = 0;
	PSG.regs[4] = 0;
	PSG.regs[5] = 0;
	PSG.regs[6] = 0;
	PSG.regs[7] = 0;
	PSG.regs[8] = 0;
	PSG.regs[9] = 0;
	PSG.regs[0xA] = 0;
	PSG.regs[0xB] = 0;
	PSG.regs[0xC] = 0;
	PSG.regs[0xD] = 0;
	PSG.regs[0xE] = 0;
	PSG.regs[0xF] = 0;

	PSG.RNG = 1;
	PSG.outputA = 0;
	PSG.outputB = 0;
	PSG.outputC = 0;
	PSG.outputN = 0xff;
	ay8910_build_mixer_table();
	PSG.ready = 1;
}

void ay8910_write(int r, int v) {
	int old;

	if (PSG.regs == NULL)
		return;
	PSG.regs[r] = v;

	/* A note about the period of tones, noise and envelope: for speed reasons,*/
	/* we count down from the period to 0, but careful studies of the chip     */
	/* output prove that it instead counts up from 0 until the counter becomes */
	/* greater or equal to the period. This is an important difference when the*/
	/* program is rapidly changing the period to modulate the sound.           */
	/* To compensate for the difference, when the period is changed we adjust  */
	/* our internal counter.                                                   */
	/* Also, note that period = 0 is the same as period = 1. This is mentioned */
	/* in the YM2203 data sheets. However, this does NOT apply to the Envelope */
	/* period. In that case, period = 0 is half as period = 1. */
	switch (r)
	{
	case AY_AFINE:
	case AY_ACOARSE:
		PSG.regs[AY_ACOARSE] &= 0x0f;
		old = PSG.periodA;
		PSG.periodA = (PSG.regs[AY_AFINE] + 256 * PSG.regs[AY_ACOARSE]) * STEP3;
		if (PSG.periodA == 0) PSG.periodA = STEP3;
		PSG.countA += PSG.periodA - old;
		if (PSG.countA <= 0) PSG.countA = 1;
		break;
	case AY_BFINE:
	case AY_BCOARSE:
		PSG.regs[AY_BCOARSE] &= 0x0f;
		old = PSG.periodB;
		PSG.periodB = (PSG.regs[AY_BFINE] + 256 * PSG.regs[AY_BCOARSE]) * STEP3;
		if (PSG.periodB == 0) PSG.periodB = STEP3;
		PSG.countB += PSG.periodB - old;
		if (PSG.countB <= 0) PSG.countB = 1;
		break;
	case AY_CFINE:
	case AY_CCOARSE:
		PSG.regs[AY_CCOARSE] &= 0x0f;
		old = PSG.periodC;
		PSG.periodC = (PSG.regs[AY_CFINE] + 256 * PSG.regs[AY_CCOARSE]) * STEP3;
		if (PSG.periodC == 0) PSG.periodC = STEP3;
		PSG.countC += PSG.periodC - old;
		if (PSG.countC <= 0) PSG.countC = 1;
		break;
	case AY_NOISEPER:
		PSG.regs[AY_NOISEPER] &= 0x1f;
		old = PSG.periodN;
		PSG.periodN = PSG.regs[AY_NOISEPER] * STEP3;
		if (PSG.periodN == 0) PSG.periodN = STEP3;
		PSG.countN += PSG.periodN - old;
		if (PSG.countN <= 0) PSG.countN = 1;
		break;
	case AY_ENABLE:
		PSG.lastEnable = PSG.regs[AY_ENABLE];
		break;
	case AY_AVOL:
		PSG.regs[AY_AVOL] &= 0x1f;
		PSG.envelopeA = PSG.regs[AY_AVOL] & 0x10;
		PSG.volA = PSG.envelopeA ? PSG.volE : PSG.volTable[PSG.regs[AY_AVOL] ? PSG.regs[AY_AVOL] * 2 + 1 : 0];
		break;
	case AY_BVOL:
		PSG.regs[AY_BVOL] &= 0x1f;
		PSG.envelopeB = PSG.regs[AY_BVOL] & 0x10;
		PSG.volB = PSG.envelopeB ? PSG.volE : PSG.volTable[PSG.regs[AY_BVOL] ? PSG.regs[AY_BVOL] * 2 + 1 : 0];
		break;
	case AY_CVOL:
		PSG.regs[AY_CVOL] &= 0x1f;
		PSG.envelopeC = PSG.regs[AY_CVOL] & 0x10;
		PSG.volC = PSG.envelopeC ? PSG.volE : PSG.volTable[PSG.regs[AY_CVOL] ? PSG.regs[AY_CVOL] * 2 + 1 : 0];
		break;
	case AY_EFINE:
	case AY_ECOARSE:
		old = PSG.periodE;
		PSG.periodE = ((PSG.regs[AY_EFINE] + 256 * PSG.regs[AY_ECOARSE])) * STEP3;
		//if (PSG.PeriodE == 0) PSG.PeriodE = STEP3 / 2;
		if (PSG.periodE == 0) PSG.periodE = STEP3;
		PSG.countE += PSG.periodE - old;
		if (PSG.countE <= 0) PSG.countE = 1;
		break;
	case AY_ESHAPE:
		/* envelope shapes:
		C AtAlH
		0 0 x x  \___

		0 1 x x  /___

		1 0 0 0  \\\\

		1 0 0 1  \___

		1 0 1 0  \/\/
				  ___
		1 0 1 1  \

		1 1 0 0  ////
				  ___
		1 1 0 1  /

		1 1 1 0  /\/\

		1 1 1 1  /___

		The envelope counter on the AY-3-8910 has 16 steps. On the YM2149 it
		has twice the steps, happening twice as fast. Since the end result is
		just a smoother curve, we always use the YM2149 behaviour.
		*/
		PSG.regs[AY_ESHAPE] &= 0x0f;
		PSG.attack = (PSG.regs[AY_ESHAPE] & 0x04) ? 0x1f : 0x00;
		if ((PSG.regs[AY_ESHAPE] & 0x08) == 0)
		{
			/* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
			PSG.hold = 1;
			PSG.alternate = PSG.attack;
		}
		else
		{
			PSG.hold = PSG.regs[AY_ESHAPE] & 0x01;
			PSG.alternate = PSG.regs[AY_ESHAPE] & 0x02;
		}
		PSG.countE = PSG.periodE;
		PSG.countEnv = 0x1f;
		PSG.holding = 0;
		PSG.volE = PSG.volTable[PSG.countEnv ^ PSG.attack];
		if (PSG.envelopeA) PSG.volA = PSG.volE;
		if (PSG.envelopeB) PSG.volB = PSG.volE;
		if (PSG.envelopeC) PSG.volC = PSG.volE;
		break;
	case AY_PORTA:
		break;
	case AY_PORTB:
		break;
	}
}

static void ay8910_callback(void* userdata, Uint8* stream, int length) {
	(void)userdata;

	int outn;
	Uint8* buf1 = stream;

	/* hack to prevent us from hanging when starting filtered outputs */
	if (!PSG.ready)
	{
		memset(stream, 0, length * sizeof(*stream));
		return;
	}

	length = length * 2;

	/* The 8910 has three outputs, each output is the mix of one of the three */
	/* tone generators and of the (single) noise generator. The two are mixed */
	/* BEFORE going into the DAC. The formula to mix each channel is: */
	/* (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable). */
	/* Note that this means that if both tone and noise are disabled, the output */
	/* is 1, not 0, and can be modulated changing the volume. */


	/* If the channels are disabled, set their output to 1, and increase the */
	/* counter, if necessary, so they will not be inverted during this update. */
	/* Setting the output to 1 is necessary because a disabled channel is locked */
	/* into the ON state (see above); and it has no effect if the volume is 0. */
	/* If the volume is 0, increase the counter, but don't touch the output. */
	if (PSG.regs[AY_ENABLE] & 0x01)
	{
		if (PSG.countA <= STEP2) PSG.countA += STEP2;
		PSG.outputA = 1;
	}
	else if (PSG.regs[AY_AVOL] == 0)
	{
		/* note that I do count += length, NOT count = length + 1. You might think */
		/* it's the same since the volume is 0, but doing the latter could cause */
		/* interferencies when the program is rapidly modulating the volume. */
		if (PSG.countA <= STEP2) PSG.countA += STEP2;
	}
	if (PSG.regs[AY_ENABLE] & 0x02)
	{
		if (PSG.countB <= STEP2) PSG.countB += STEP2;
		PSG.outputB = 1;
	}
	else if (PSG.regs[AY_BVOL] == 0)
	{
		if (PSG.countB <= STEP2) PSG.countB += STEP2;
	}
	if (PSG.regs[AY_ENABLE] & 0x04)
	{
		if (PSG.countC <= STEP2) PSG.countC += STEP2;
		PSG.outputC = 1;
	}
	else if (PSG.regs[AY_CVOL] == 0)
	{
		if (PSG.countC <= STEP2) PSG.countC += STEP2;
	}

	/* for the noise channel we must not touch OutputN - it's also not necessary */
	/* since we use outn. */
	if ((PSG.regs[AY_ENABLE] & 0x38) == 0x38)	/* all off */
		if (PSG.countN <= STEP2) PSG.countN += STEP2;

	outn = (PSG.outputN | PSG.regs[AY_ENABLE]);

	/* buffering loop */
	while (length > 0)
	{
		unsigned vol;
		int left = 2;
		/* vola, volb and volc keep track of how long each square wave stays */
		/* in the 1 position during the sample period. */

		int vola, volb, volc;
		vola = volb = volc = 0;

		do
		{
			int nextevent;

			if (PSG.countN < left) nextevent = PSG.countN;
			else nextevent = left;

			if (outn & 0x08)
			{
				if (PSG.outputA) 
					vola += PSG.countA;

				PSG.countA -= nextevent;
				/* PeriodA is the half period of the square wave. Here, in each */
				/* loop I add PeriodA twice, so that at the end of the loop the */
				/* square wave is in the same status (0 or 1) it was at the start. */
				/* vola is also incremented by PeriodA, since the wave has been 1 */
				/* exactly half of the time, regardless of the initial position. */
				/* If we exit the loop in the middle, OutputA has to be inverted */
				/* and vola incremented only if the exit status of the square */
				/* wave is 1. */
				while (PSG.countA <= 0)
				{
					PSG.countA += PSG.periodA;
					if (PSG.countA > 0)
					{
						PSG.outputA ^= 1;
						if (PSG.outputA) 
							vola += PSG.periodA;
						break;
					}
					PSG.countA += PSG.periodA;
					vola += PSG.periodA;
				}
				if (PSG.outputA) vola -= PSG.countA;
			}
			else
			{
				PSG.countA -= nextevent;
				while (PSG.countA <= 0)
				{
					PSG.countA += PSG.periodA;
					if (PSG.countA > 0)
					{
						PSG.outputA ^= 1;
						break;
					}
					PSG.countA += PSG.periodA;
				}
			}

			if (outn & 0x10)
			{
				if (PSG.outputB) volb += PSG.countB;
				PSG.countB -= nextevent;
				while (PSG.countB <= 0)
				{
					PSG.countB += PSG.periodB;
					if (PSG.countB > 0)
					{
						PSG.outputB ^= 1;
						if (PSG.outputB) volb += PSG.periodB;
						break;
					}
					PSG.countB += PSG.periodB;
					volb += PSG.periodB;
				}
				if (PSG.outputB) volb -= PSG.countB;
			}
			else
			{
				PSG.countB -= nextevent;
				while (PSG.countB <= 0)
				{
					PSG.countB += PSG.periodB;
					if (PSG.countB > 0)
					{
						PSG.outputB ^= 1;
						break;
					}
					PSG.countB += PSG.periodB;
				}
			}

			if (outn & 0x20)
			{
				if (PSG.outputC) volc += PSG.countC;
				PSG.countC -= nextevent;
				while (PSG.countC <= 0)
				{
					PSG.countC += PSG.periodC;
					if (PSG.countC > 0)
					{
						PSG.outputC ^= 1;
						if (PSG.outputC) volc += PSG.periodC;
						break;
					}
					PSG.countC += PSG.periodC;
					volc += PSG.periodC;
				}
				if (PSG.outputC) volc -= PSG.countC;
			}
			else
			{
				PSG.countC -= nextevent;
				while (PSG.countC <= 0)
				{
					PSG.countC += PSG.periodC;
					if (PSG.countC > 0)
					{
						PSG.outputC ^= 1;
						break;
					}
					PSG.countC += PSG.periodC;
				}
			}

			PSG.countN -= nextevent;
			if (PSG.countN <= 0)
			{
				/* Is noise output going to change? */
				if ((PSG.RNG + 1) & 2)	/* (bit0^bit1)? */
				{
					PSG.outputN = ~PSG.outputN;
					outn = (PSG.outputN | PSG.regs[AY_ENABLE]);
				}

				/* The Random Number Generator of the 8910 is a 17-bit shift */
				/* register. The input to the shift register is bit0 XOR bit3 */
				/* (bit0 is the output). This was verified on AY-3-8910 and YM2149 chips. */

				/* The following is a fast way to compute bit17 = bit0^bit3. */
				/* Instead of doing all the logic operations, we only check */
				/* bit0, relying on the fact that after three shifts of the */
				/* register, what now is bit3 will become bit0, and will */
				/* invert, if necessary, bit14, which previously was bit17. */
				if (PSG.RNG & 1) PSG.RNG ^= 0x24000; /* This version is called the "Galois configuration". */
				PSG.RNG >>= 1;
				PSG.countN += PSG.periodN;
			}

			left -= nextevent;
		} while (left > 0);

		/* update envelope */
		if (PSG.holding == 0)
		{
			PSG.countE -= STEP;
			if (PSG.countE <= 0)
			{
				do
				{
					PSG.countEnv--;
					PSG.countE += PSG.periodE;
				} while (PSG.countE <= 0);

				/* check envelope current position */
				if (PSG.countEnv < 0)
				{
					if (PSG.hold)
					{
						if (PSG.alternate)
							PSG.attack ^= 0x1f;
						PSG.holding = 1;
						PSG.countEnv = 0;
					}
					else
					{
						/* if CountEnv has looped an odd number of times (usually 1), */
						/* invert the output. */
						if (PSG.alternate && (PSG.countEnv & 0x20))
							PSG.attack ^= 0x1f;

						PSG.countEnv &= 0x1f;
					}
				}

				PSG.volE = PSG.volTable[PSG.countEnv ^ PSG.attack];
				/* reload volume */
				if (PSG.envelopeA) PSG.volA = PSG.volE;
				if (PSG.envelopeB) PSG.volB = PSG.volE;
				if (PSG.envelopeC) PSG.volC = PSG.volE;
			}
		}

		vol = (vola * PSG.volA + volb * PSG.volB + volc * PSG.volC) / (3 * STEP);
		if (--length & 1)
			*(buf1++) = vol >> 8;
	}
}

static void ay8910_build_mixer_table() {
	int i;
	double out;

	/* calculate the volume->voltage conversion table */
	/* The AY-3-8910 has 16 levels, in a logarithmic scale (3dB per STEP) */
	/* The YM2149 still has 16 levels for the tone generators, but 32 for */
	/* the envelope generator (1.5dB per STEP). */
	out = MAX_OUTPUT;
	for (i = 31; i > 0; i--)
	{
		PSG.volTable[i] = (unsigned)(out + 0.5);	/* round to nearest */
		out /= 1.188502227;	/* = 10 ^ (1.5/20) = 1.5dB */
	}
	PSG.volTable[0] = 0;
}

void ay8910_init_sound() {
	// SDL audio stuff
	SDL_AudioSpec reqSpec;
	SDL_AudioSpec givenSpec;

	resetAY8910();

	// set up audio buffering
	reqSpec.freq = SOUND_FREQ;            // Audio frequency in samples per second
	reqSpec.format = AUDIO_U8;          // Audio data format
	reqSpec.channels = 1;            // Number of channels: 1 mono, 2 stereo
	reqSpec.samples = SOUND_SAMPLE;            // Audio buffer size in samples
	reqSpec.callback = ay8910_callback;      // Callback function for filling the audio buffer
	reqSpec.userdata = NULL;
	/* Open the audio device */
	if (SDL_OpenAudio(&reqSpec, &givenSpec) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}

# if 0
	fprintf(stdout, "samples:%d format=%x freq=%d\n", givenSpec.samples, givenSpec.format, givenSpec.freq);
# endif

	// Start playing audio
	SDL_PauseAudio(0);
}

void ay8910_done_sound() {
	SDL_CloseAudio();
}