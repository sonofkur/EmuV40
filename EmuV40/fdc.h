#ifndef FDC_H
#define FDC_H

#include <stdint.h>

//82077AA floppy disk controller (FDC)
#define SRA_DIR 0x01
#define SRA_WP 0x02
#define SRA_INDX 0x04
#define SRA_HDSEL 0x08
#define SRA_TRK0 0x10
#define SRA_STEP 0x20
#define SRA_DRV2 0x40
#define SRA_INTPEND 0x80

#define SRB_MTR0 0x01
#define SRB_MTR1 0x02
#define SRB_WGATE 0x04
#define SRB_RDATA 0x08
#define SRB_WDATA 0x10
#define SRB_DR0 0x20
#define SRB_DRV2 0x80

#define SR0_EQPMT 0x10
#define SR0_SEEK 0x20
#define SR0_ABNTERM 0x40
#define SR0_INVCMD 0x80
#define SR0_RDY (SR0_ABNTERM | SR0_INVCMD)

#define DOR_MOTD 0x80
#define DOR_MOTC 0x40
#define DOR_MOTB 0x20
#define DOR_MOTA 0x10
#define DOR_IRQ 0x08
#define DOR_RESET 0x04
#define DOR_DSEL1 0x02
#define DOR_DSEL0 0x01

#define MSR_RQM 0x80
#define MSR_DIO 0x40
#define MSR_NDMA 0x20
#define MSR_CB 0x10 // Similar to IDE BSY
#define MSR_ACTD 0x08
#define MSR_ACTC 0x04
#define MSR_ACTB 0x02
#define MSR_ACTA 0x01

struct fdc {
    // <<< BEGIN STRUCT "struct" >>>
    /// ignore: dmabuf, drives
    uint8_t status[2]; // 3F0/3F1
    uint8_t dor; // 3F2
    uint8_t tape_drive[4]; // 3F3, unused
    uint8_t msr, data_rate; // 3F4
    uint8_t fifo; // 3F5
    // 3F6 is reserved for IDE
    uint8_t dir[4], ccr; // 3F7, one dir for each drive

    // Status registers
    uint8_t st[4];

    int selected_drive;

    // For read/write
    int multi_mode;

    // For format
    int format_bytes_to_read,
        format_byte,
        format_dma_pos;

    // Command bytes sent to controller
    uint8_t command_buffer[16];
    uint8_t command_buffer_size; // Number of bytes that controller wants
    uint8_t command_buffer_pos; // Number of bytes sent to controller

    // Response bytes to be read by controller
    uint8_t response_buffer[16];
    uint8_t response_buffer_size;
    uint8_t response_pos;

    // Cache seek cylinder, head, and sector, and also cache internal LBA address for fast lookups.
    uint32_t seek_cylinder[2], seek_head[2], seek_sector[2], seek_internal_lba[2];

    // Interrupt countdown, needed for command 8
    int interrupt_countdown;

    // There are two floppy ribbons, with two drives attached to each
    struct drive_info* drives[4];
    uint32_t drive_inserted[4];
    uint32_t drive_size[4];
    uint32_t drive_heads[4];
    uint32_t drive_cylinders[4];
    uint32_t drive_spt[4];
    int drive_write_protected[4];

    uint32_t write_length;

    // Can be read/written, but not used
    uint8_t precomp, config;
    uint8_t locked;
    uint8_t perpendicular_mode;

    uint8_t dmabuf[16 << 10];
    // <<< END STRUCT "struct" >>>
};

extern struct fdc fdc;

extern void initFDC();
extern uint8_t inFDC(uint16_t portnum);
extern void outFDC(uint16_t portnum, uint8_t value);

void fdc_set_st0(int bits);
int fdc_get_st0(void);
void fdc_abort_command(void);
void fdc_abort_command2(void);
void fdc_read_cb(void* a, int b);
void fdc_write_cb(void* a, int b);
void fdc_idle(void);
void fdc_handle_format(void* a, int b);
int floppy_next(uint64_t now);
void* fdc_dma_buf(void);
void fdc_dma_complete(void);
void fdc_replace_drive(int idx, struct drive_info* drive);
void fdc_init(struct pc_settings* pc);

#endif