/******************************************************************************
 * Middleware/midi_router.h
 *
 * Multi-port MIDI router with per-channel/per-message-type filtering.
 * Supports USB, UART, and Internal (virtual) ports.
 *****************************************************************************/

#ifndef MIDI_ROUTER_H
#define MIDI_ROUTER_H

#include "midi_parser.h"
#include "ring_buffer.h"

typedef enum {
    PORT_USB = 0,
    PORT_UART,
    PORT_INT,           /* Internal / virtual port */
    PORT_COUNT
} port_id_t;

typedef struct {
    port_id_t id;
    void        (*send)(const uint8_t *data, uint16_t len);
    uint16_t    (*poll)(uint8_t * buf, uint16_t max);
    ring_buf_t  *tx;
    ring_buf_t  *rx;
} midi_port_t;

/* Message type filter masks */
#define MASK_NOTE   (1U << 0)
#define MASK_CC     (1U << 1)
#define MASK_PC     (1U << 2)
#define MASK_PB     (1U << 3)
#define MASK_SX     (1U << 4)
#define MASK_RT     (1U << 5)
#define MASK_ALL    0xFFU

typedef struct {
    uint8_t  enabled;
    uint16_t ch_mask;       /* Bit0=CH1 .. Bit15=CH16 */
    uint8_t  msg_mask;
} route_rule_t;

void router_init(void);
void router_reg_port(port_id_t id, midi_port_t *port);
void router_set_rule(port_id_t src, port_id_t dst, const route_rule_t *rule);
void router_process(port_id_t src, const midi_msg_t *msg);
void router_poll_all(void);

typedef void (*midi_rx_cb_t)(port_id_t port, const midi_msg_t *msg);
void router_set_rx_cb(midi_rx_cb_t cb);

#endif /* MIDI_ROUTER_H */
