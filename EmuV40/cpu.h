#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#define regax 0
#define regcx 1
#define regdx 2
#define regbx 3
#define regsp 4
#define regbp 5
#define regsi 6
#define regdi 7
#define reges 0
#define regcs 1
#define regss 2
#define regds 3

#define regal 0
#define regah 1
#define regcl 2
#define regch 3
#define regdl 4
#define regdh 5
#define regbl 6
#define regbh 7

#define RAM_SIZE 0x100000

union _bytewordregs_ {
	uint16_t wordregs[8];
	uint8_t byteregs[8];
};

typedef struct _cpu_ {
	uint8_t RAM[RAM_SIZE];
	uint8_t readonly[RAM_SIZE];

	uint8_t     cf, zf, pf, af, sf, tf, ifl, df, of;
	uint16_t	savecs, saveip, ip, useseg;
	int         hltstate;
	uint16_t    segregs[4];
	uint8_t		segoverride;
	uint8_t		nmi;
	union       _bytewordregs_ regs;

	uint32_t cycles;
	uint8_t running;
	uint32_t makeupticks;
	uint64_t totalexec;
	uint32_t clockticks;
	float MHZ;
	uint16_t cpu_last_int_seg, cpu_last_int_ip;
	uint8_t verbose, didbootstrap;
}CPU;

extern CPU cpu;

static inline uint16_t makeflagsword(void)
{
	return 2 | (uint16_t)cpu.cf | ((uint16_t)cpu.pf << 2) | ((uint16_t)cpu.af << 4) | ((uint16_t)cpu.zf << 6) | ((uint16_t)cpu.sf << 7) |
		((uint16_t)cpu.tf << 8) | ((uint16_t)cpu.ifl << 9) | ((uint16_t)cpu.df << 10) | ((uint16_t)cpu.of << 11)
		;
}

static inline void decodeflagsword(uint16_t x)
{
	cpu.cf = x & 1;
	cpu.pf = (x >> 2) & 1;
	cpu.af = (x >> 4) & 1;
	cpu.zf = (x >> 6) & 1;
	cpu.sf = (x >> 7) & 1;
	cpu.tf = (x >> 8) & 1;
	cpu.ifl = (x >> 9) & 1;
	cpu.df = (x >> 10) & 1;
	cpu.of = (x >> 11) & 1;
}

#define CPU_FL_CF	cpu.cf
#define CPU_FL_PF	cpu.pf
#define CPU_FL_AF	cpu.af
#define CPU_FL_ZF	cpu.zf
#define CPU_FL_SF	cpu.sf
#define CPU_FL_TF	cpu.tf
#define CPU_FL_IFL	cpu.ifl
#define CPU_FL_DF	cpu.df
#define CPU_FL_OF	cpu.of

#define CPU_CS		cpu.segregs[regcs]
#define CPU_DS		cpu.segregs[regds]
#define CPU_ES		cpu.segregs[reges]
#define CPU_SS		cpu.segregs[regss]

#define CPU_AX  	cpu.regs.wordregs[regax]
#define CPU_BX  	cpu.regs.wordregs[regbx]
#define CPU_CX  	cpu.regs.wordregs[regcx]
#define CPU_DX  	cpu.regs.wordregs[regdx]
#define CPU_SI  	cpu.regs.wordregs[regsi]
#define CPU_DI  	cpu.regs.wordregs[regdi]
#define CPU_BP  	cpu.regs.wordregs[regbp]
#define CPU_SP  	cpu.regs.wordregs[regsp]
#define CPU_IP		cpu.ip

#define CPU_AL  	cpu.regs.byteregs[regal]
#define CPU_BL  	cpu.regs.byteregs[regbl]
#define CPU_CL  	cpu.regs.byteregs[regcl]
#define CPU_DL  	cpu.regs.byteregs[regdl]
#define CPU_AH  	cpu.regs.byteregs[regah]
#define CPU_BH  	cpu.regs.byteregs[regbh]
#define CPU_CH  	cpu.regs.byteregs[regch]
#define CPU_DH  	cpu.regs.byteregs[regdh]

static inline void write_mem8(uint32_t addr32, uint8_t value);
static inline void write_mem16(uint32_t addr32, uint16_t value);
static inline uint8_t  read_mem8(uint32_t addr32);
static inline uint16_t read_mem16(uint32_t addr32);
uint8_t  read_mem_offset(uint16_t seg, uint16_t off);


void     cpu_reset(void);
uint32_t cpu_exec(uint32_t execloops);
void     cpu_push(uint16_t pushval);
uint16_t cpu_pop(void);
void     cpu_iret(void);
int      cpu_hlt_handler(void);

#define ABS_TO_SEGMENT(a) ((a >> 4) & 0xFFFF)
#define ABS_TO_OFFSET(a) (a & 0xFFFF)

#endif
