
/******************************************************************************
 * Middleware/dfu_engine.h
 *
 * Application-mode DFU via SysEx.
 * Host sends firmware blocks, we verify CRC32 and program Flash.
 *****************************************************************************/

#ifndef DFU_ENGINE_H
#define DFU_ENGINE_H
#include "common_types.h"
typedef enum {
    DFU_IDLE = 0, 
    DFU_RECEIVING,
    DFU_VERIFYING,
    DFU_PROGRAMMING,
    DFU_COMPLETE,
    DFU_ERROR
} dfu_state_t;

/* SysEx DFU commands */
#define DFU_CMD_START   0x01
#define DFU_CMD_DATA    0x02
#define DFU_CMD_VERIFY  0x03
#define DFU_CMD_REBOOT  0x04
#define DFU_BLOCK_SIZE  128U

typedef struct {
    dfu_state_t state;
    uint32_t    total_size;
    uint32_t    received;
    uint32_t    expected_crc;
    uint32_t    running_crc;
    uint16_t    next_block;
    uint8_t     page_buf[FMC_PAGE_SIZE];
    uint16_t    page_offset;
    uint32_t    flash_addr;
} dfu_ctx_t;



#endif /* DFU_ENGINE_H */


