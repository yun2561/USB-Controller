/******************************************************************************
 * App/usb_midi_bridge.c
 *
 * RX: USB ISR → rb_push(g_usb_rx) → poll decodes CIN → raw bytes → router
 * TX: router → usb_port_send() handles both channel msgs and SysEx
 *     - Channel/RT bytes: queued in g_usb_tx → drain re-encodes
 *     - SysEx (F0..F7):   encoded + sent immediately via enc_sysex
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
}

/* ==================================================================
 * TX: unified send — router calls this for both channel and SysEx
 *
 * SysEx detection: if data[0] == 0xF0, treat entire buffer as a
 * complete SysEx message and encode via usb_midi_enc_sysex() directly.
 * Otherwise, queue raw bytes for drain to re-parse and encode.
 * ================================================================== */
static uint16_t usb_port_poll(uint8_t *buf, uint16_t max)
{
    uint16_t out = 0;
    uint8_t pkt[4];
    while (out + 3 <= max && rb_avail(&g_usb_rx) >=4) {
        rb_pop_blk(&g_usb_rx, pkt, 4);
        uint8_t cin = pkt[0] & 0x0F;
        switch (cin)
        {
        case 0x04:
            buf[out++] = pkt[1];
            buf[out++] = pkt[2];
            buf[out++] = pkt[3];
            break;

        /* SysEx end with 3 bytes */
        case 0x07:
            buf[out++] = pkt[1];
            buf[out++] = pkt[2];
            buf[out++] = pkt[3];
            break;
        
        /* SysEx end with 2 bytes */
        case 0x06:
            buf[out++] = pkt[1];
            buf[out++] = pkt[2];
            break;

        /* SysEx end with 1 byte */
        case 0x05:
            if (pkt[1] == 0xF7 || pkt[1] == 0xF0) {
                buf[out++] = pkt[1];
            }
            break;

        default: {
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
                break;
            } 
        }
    }
    return out;
}   


/* ==================================================================
 * TX: unified send — router calls this for both channel and SysEx
 *
 * SysEx detection: if data[0] == 0xF0, treat entire buffer as a
 * complete SysEx message and encode via usb_midi_enc_sysex() directly.
 * Otherwise, queue raw bytes for drain to re-parse and encode.
 * ================================================================== */
static void usb_port_send(const uint8_t *data, uint16_t len)
{
    if (len == 0) return;
    /* ---- SysEx: encode and send immediately ---- */
    if (data[0] == 0xF0) {
        if (!bsp_usb_is_configured()) return;
        usb_midi_pkt_t pkts[(SX_MAX_LEN / 3) + 2];
        uint16_t npkt = usb_midi_enc_sysex(data, len, 0, pkts, sizeof(pkts) / sizeof(pkts));

        uint16_t idx = 0;
        while (idx < npkt)
        {
            uint32_t timeout = 50000;
            while (!bsp_usb_tx_ready() && --timeout) ;
            if (!timeout) return;
            uint8_t frame[MIDI_EP_MAX_PKT];
            uint16_t flen = 0;
            while (flen + 4 <= MIDI_EP_MAX_PKT && idx < npkt){
                frame[flen++] = pkts[idx].cin_cable;
                frame[flen++] = pkts[idx].midi[0];
                frame[flen++] = pkts[idx].midi[1];
                frame[flen++] = pkts[idx].midi[2];
                idx++;
            }
            bsp_usb_send(frame, len);
            
        }
        return ;
        
    }
    rb_push_blk(&g_usb_tx, data, len);
}

/* ==================================================================
 * TX drain: main loop — re-parses queued channel/RT bytes →
 * USB-MIDI packets → bsp_usb_send()
 * (SysEx never enters g_usb_tx — already sent in usb_port_send)
 * ================================================================== */
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

    while (pkt_len + 4 <= MIDI_EP_MAX_PKT && rb_avail(&g_usb_tx) > 0) {
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
