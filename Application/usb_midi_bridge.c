/******************************************************************************
 * App/usb_midi_bridge.c
 *****************************************************************************/
#include "usb_midi_bridge.h"
#include "bsp_usb_midi.h"
#include "ring_buffer.h"
#include "usb_midi_codec.h"
#include "midi_parser.h"
#include "midi_router.h"

/* Ring buffers (defined in main.c, extern here) */
extern ring_buf_t g_usb_tx;
extern ring_buf_t g_usb_rx;
#define MIDI_EP_MAX_PKT    64

/* ---- RX: ISR callback → ring buffer ---- */
static void usb_rx_isr_cb(const uint8_t *data, uint16_t len)
{
    uint16_t pushed = rb_push_blk(&g_usb_rx, data, len);
    if (pushed < len) {
        ;
    }
}

/* ---- RX: poll function for router ----
 * Reads 4-byte USB-MIDI packets from g_usb_rx,
 * decodes CIN → raw MIDI bytes for parser.
 */
static uint16_t usb_port_poll(uint8_t *buf, uint16_t max)
{
    uint16_t out = 0;
    uint8_t pkt[4];
    while (out + 3 <= max && rb_avail(&g_usb_rx) >=4){
        rb_pop_blk(&g_usb_rx, pkt, 4);
        usb_midi_pkt_t p = {
            .cin_cable = pkt[0],
            .midi = { pkt[1], pkt[2], pkt[3] }
        };

        midi_msg_t msg;
        if (usb_midi_dec(&p, &msg)) {
            buf[out++] = msg.status;
            if (msg.length >= 2) buf[out++] = msg.data[0];
            if (msg.length >= 3) buf[out++] = msg.data[1];
        }
    }
    return out;
}

/* ---- TX: send function for router ----
 * Router calls this with raw MIDI bytes; we queue them.
 */
static void usb_port_send(const uint8_t *data, uint16_t len)
{
    uint16_t pushed = rb_push_blk(&g_usb_tx, data, len);
    if (pushed < len) {
        ;;;
    }
}

/* ---- TX drain: main loop calls this ----
 * Re-parses queued bytes → USB-MIDI packets → bsp_usb_send().
 */
static midi_parser_t s_tx_parser;
static uint8_t       s_tx_parser_inited = 0;

void usb_bridge_drain_tx(void)
{
    if (!bsp_usb_is_configured()) return;
    if (!bsp_usb_tx_ready()) return;
    if (rb_avail(&g_usb_tx) == 0) return;

    if (!s_tx_parser_inited) {
        midi_parser_init(&s_tx_parser);
        s_tx_parser_inited = 1;
    }

    uint8_t usb_pkt[MIDI_EP_MAX_PKT];
    uint16_t pkt_len = 0;

    while (pkt_len + 4 <= MIDI_EP_MAX_PKT && rb_avail(&g_usb_tx) > 0){
        uint8_t byte;
        if (!rb_pop(&g_usb_tx, &byte)) break;

        midi_msg_t msg;
        if (midi_parse(&s_tx_parser, byte, &msg)) {
            usb_midi_pkt_t p;
            if (usb_midi_enc(&msg, 0, &p)) {
                usb_pkt[pkt_len++] = p.cin_cable;
                usb_pkt[pkt_len++] = p.midi[0];
                usb_pkt[pkt_len++] = p.midi[1];
                usb_pkt[pkt_len++] = p.midi[2];
            }
        }
    }
    if (pkt_len > 0) {
        bsp_usb_send(usb_pkt,pkt_len);
    }

}

/* ---- Init ---- */
static midi_port_t s_usb_port;

void usb_bridge_init(void)
{
    rb_init(&g_usb_rx);
    rb_init(&g_usb_tx);

    bsp_usb_set_rx_callback(usb_rx_isr_cb);

    s_usb_port = (midi_port_t){
        .id   = PORT_USB,
        .send = usb_port_send,
        .poll = usb_port_poll,
        .tx   = &g_usb_tx,
        .rx   = &g_usb_rx,
    };

    router_reg_port(PORT_USB,&s_usb_port);

    s_tx_parser_inited = 0;
}
