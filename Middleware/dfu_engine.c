/******************************************************************************
 * Middleware/dfu_engine.c
 *****************************************************************************/
#include "dfu_engine.h"
#include "crc_engine.h"
#include "bsp_fmc.h"
#include "bsp_system.h"
#include <string.h>

static dfu_progress_cb_t s_pcb = NULL;
static dfu_complete_cb_t s_ccb = NULL;

void dfu_init(dfu_ctx_t *ctx)
{
    memset(ctx, 0 , sizeof(*ctx));
}

void dfu_set_callbacks(dfu_progress_cb_t pcb, dfu_complete_cb_t ccb)
{
    s_pcb = pcb;
    s_ccb = ccb;
}

uint8_t dfu_progress(const dfu_ctx_t *ctx)
{
    return ctx->total_size ? (uint8_t)(ctx->received * 100 / ctx->total_size) :0;
}

static void flush_page(dfu_ctx_t *ctx)
{
    bsp_fmc_erase(ctx->flash_addr);
    bsp_fmc_prog(ctx->flash_addr,(const uint32_t *)ctx->page_buf, ctx->page_offset / 4);
    ctx->flash_addr += FMC_PAGE_SIZE;
    ctx->page_offset = 0;
    memset(ctx->page_buf, 0xFF, FMC_PAGE_SIZE);
}

void dfu_process_sysex(dfu_ctx_t *ctx, const uint8_t *d, uint16_t len)
{
    if (len < 7) return ;
    uint8_t cmd = d[4];

    switch (cmd)
    {
        case DFU_CMD_START:
            if(len < 13) return ;
            ctx->total_size     = ((uint32_t)d[5]<<24) | ((uint32_t)d[6]<<16) |
                                  ((uint32_t)d[7]<<8)  | d[8];
            ctx->expected_crc   = ((uint32_t)d[9]<<24) | ((uint32_t)d[10]<<16)|
                                  ((uint32_t)d[11]<<8) | d[12];
            if (ctx->total_size > (CONFIG_ADDR - APP_ADDR)){
                ctx->state = DFU_ERROR;
                return;
            }
            ctx->state          = DFU_RECEIVING;
            ctx->received       = 0;
            ctx->running_crc    = 0;
            ctx->next_block     = 0;
            ctx->page_offset    = 0;
            ctx->flash_addr     = APP_ADDR;
            memset(ctx->page_buf, 0xFF, FMC_PAGE_SIZE);
            break;
        
        case DFU_CMD_DATA:
            if (ctx->state != DFU_RECEIVING || len < 8) return ;
            {
                uint16_t blk    = ((uint16_t)d[5] << 8) | d[6];
                if (blk != ctx->next_block) { ctx->state = DFU_ERROR; return; }

                uint16_t data_len = len - 8;
                const uint8_t *payload = &d[7];

                for (uint16_t i = 0; i < data_len; i++){
                    ctx->page_buf[ctx->page_offset++]   = payload[i];
                    ctx->received++;
                    ctx->running_crc = crc32(&payload[i], 1, ctx->running_crc);

                    if (ctx->page_offset >= FMC_PAGE_SIZE)
                        flush_page(ctx);
                }
                ctx->next_block++;
                if (s_pcb) s_pcb(dfu_progress(ctx));
            }
            break;
        
        case DFU_CMD_VERIFY:
            if (ctx->state != DFU_RECEIVING) return ;
            /* Flush remaining partial page */
            if (ctx->page_offset > 0){
                while (ctx->page_offset % 4) ctx->page_buf[ctx->page_offset++] = 0xFF;
                flush_page(ctx);
            }
            if (ctx->running_crc == ctx->expected_crc) {
                ctx->state = DFU_COMPLETE;
                if (s_ccb) s_ccb(1);
            } else {
                ctx->state = DFU_ERROR;
                if (s_ccb) s_ccb(0);
            }
            break;
        
        case DFU_CMD_REBOOT: {
            uint32_t flag = DFU_MAGIC_NORMAL;
            bsp_fmc_erase(DFU_STAGING_ADDR);
            bsp_fmc_prog(DFU_STAGING_ADDR, &flag, 1);
            bsp_system_reset();
        }
            
    }
}

