/******************************************************************************
 * Middleware/sysex_assembler.h
 *
 * Reassembles SysEx messages (F0..F7) from byte stream.
 * Handles multi-packet USB-MIDI and UART SysEx.
 * Timeout aborts incomplete messages.
 *****************************************************************************/
 #ifndef SYSEX_ASSEMBLER_H
 #define SYSEX_ASSEMBLER_H

#include "common_types.h"
#define SX_MAX_LEN  512U
#define SX_TIMEOUT  300U /* ms */

typedef enum
{
    SX_IDLE,
    SX_RECEIVING,
    SX_COMPLETE,
    SX_ERR_OVERFLOW,
    SX_ERR_TIMEOUT
} sx_state_t;

typedef struct {
    sx_state_t  state;
    uint8_t     buf[SX_MAX_LEN];
    uint16_t    len;
    uint32_t    timestamp;
} sysex_ctx_t;

void    sx_init(sysex_ctx_t *ctx);

void    sx_feed(sysex_ctx_t *ctx,uint8_t byte, uint32_t now_ms);

uint8_t sx_is_complete(const sysex_ctx_t *ctx);

void    sx_reset(sysex_ctx_t *ctx);

typedef void (*sx_complete_cb_t)(const uint8_t *data, uint16_t len);

void    sx_set_cb(sx_complete_cb_t cb);

#endif /* SYSEX_ASSEMBLER_H */


