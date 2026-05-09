/******************************************************************************
 * App/uart_midi_bridge.c
 *
 * RX: DMA circular → poll reads bytes → router (parser + SysEx assembler)
 * TX: router → send() → ring buffer → DMA
 * UART is a raw byte stream — channel msgs and SysEx F0..F7 both go
 * through the same send() with no special encoding.
 *****************************************************************************/
#include "uart_midi_bridge.h"
#include "bsp_uart_midi.h"
#include "ring_buffer.h"
#include "midi_router.h"

static ring_buf_t s_uart_tx;

/* ---- RX: poll from DMA circular buffer ---- */
static uint16_t uart_port_poll(uint8_t *buf, uint16_t max)
{
    uint16_t avail = bsp_uart_midi_rx_available();
    uint16_t n = 0;
    while (n < max && n < avail) {
        buf[n++] = bsp_uart_midi_rx_read();
    }
    return n;
}

/* ---- TX: send raw bytes (channel, RT, and SysEx — all the same) ---- */
static void uart_port_send(const uint8_t *data, uint16_t len)
{
    rb_push_blk(&s_uart_tx, data, len);
}

/* ---- TX drain: push queued bytes to DMA ---- */
void uart_bridge_drain_tx(void)
{
    if (bsp_uart_midi_tx_busy()) return;

    uint16_t avail = rb_avail(&s_uart_tx);
    if (avail == 0) return;

    uint8_t tmp[UART_TX_BUF_SIZE];
    uint16_t len = rb_pop_blk(&s_uart_tx, tmp, avail);
    if (len > 0)
        bsp_uart_midi_send_dma(tmp,len);

}

/* ---- Init ---- */
static midi_port_t s_uart_port;

void uart_bridge_init(void)
{
    rb_init(&s_uart_tx);

    s_uart_port = (midi_port_t) {
        .id     = PORT_UART,
        .send   = uart_port_send, /* Channel + SysEx, no distinction */
        .poll   = uart_port_poll,
        .tx     = &s_uart_tx,
        .rx     = NULL,
    };

    router_reg_port(PORT_UART, &s_uart_port);
}



