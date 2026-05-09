/******************************************************************************
 * Middleware/sysex_assembler.c
 *****************************************************************************/
#include "sysex_assembler.h"
#include <string.h>

static sx_complete_cb_t s_cb = NULL;

void sx_init(sysex_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
}

void sx_set_cb(sx_complete_cb_t cb)
{
    s_cb = cb;
}

void sx_feed(sysex_ctx_t *ctx, uint8_t b, uint32_t now)
{
    switch (ctx->state)
    {
        case SX_IDLE:
            if (b == 0xF0){
                ctx->state  = SX_RECEIVING;
                ctx->len    = 0;
                ctx->buf[ctx->len++] = b;
                ctx->timestamp = now;
            }
            break;
        
        case SX_RECEIVING:
            /* Timeout check */
            if ((now - ctx->timestamp) >SX_TIMEOUT){
                ctx->state = SX_ERR_TIMEOUT;
                return;
            }

            if (b== 0xF7){
                /* End of SysEx */
                if (ctx->len < SX_MAX_LEN)
                    ctx->buf[ctx->len++] = b;
                ctx->state = SX_COMPLETE;
                if(s_cb) s_cb(ctx->buf, ctx->len);
            }

            else if (b & 0x80) {
                /* Non-realtime status inside SysEx — abort */
                if (b < 0xF8) ctx->state = SX_IDLE;
                /* Real-time (≥0xF8) can appear inside SysEx — ignore */
            }
            else {
                /* Data byte */
                if (ctx->len < SX_MAX_LEN) {
                    ctx->buf[ctx->len++] = b;
                    ctx->timestamp       = now; /* Reset timeout */
                } else {
                    ctx->state = SX_ERR_OVERFLOW;
                }
            }
            break;

        case SX_COMPLETE:
        case SX_ERR_OVERFLOW:
        case SX_ERR_TIMEOUT:
            /* Must call sx_reset() to accept new data */
            break;      
    }
}

uint8_t sx_is_complete(const sysex_ctx_t *ctx)
{
    return (ctx->state == SX_COMPLETE) ? 1 : 0;
}

void sx_reset(sysex_ctx_t *ctx)
{
    ctx->state = SX_IDLE;
    ctx->len   = 0;
}

