/******************************************************************************
 * Middleware/midi_router.c
 *****************************************************************************/
#include "midi_router.h"

#include <string.h>

static midi_port_t      *s_ports[PORT_COUNT];
static route_rule_t     s_rules[PORT_COUNT][PORT_COUNT];
static midi_rx_cb_t     s_rx_cb = NULL;
static midi_parser_t    s_parsers[PORT_COUNT];

void router_init(void)
{
    memset(s_ports, 0, sizeof(s_ports));
    memset(s_rules, 0, sizeof(s_rules));

    /* Default: USB ↔ UART bidirectional, all channels, all types */
    s_rules[PORT_USB][PORT_UART]  = (route_rule_t){ 1, 0xFFFF, MASK_ALL };
    s_rules[PORT_UART][PORT_USB]  = (route_rule_t){ 1, 0xFFFF, MASK_ALL };

    for (int i = 0; i < PORT_COUNT; i++)
        midi_parser_init(&s_parsers[i]);

}

void router_reg_port(port_id_t id, midi_port_t *port)
{
    if (id < PORT_COUNT) s_ports[id] = port;
}

void router_set_rule(port_id_t src, port_id_t dst, const route_rule_t *rule)
{
    if (src < PORT_COUNT && dst < PORT_COUNT)
        s_rules[src][dst] = *rule;
}

void router_set_rx_cb(midi_rx_cb_t cb) 
{
    s_rx_cb = cb;
}

static uint8_t msg_matches(const midi_msg_t *m,const route_rule_t *r)
{
    if (!r->enabled) return 0;
    /* Channel filter (for channel messages only) */
    if (m->status < 0xF0){
        if (!(r->ch_mask & (1U << m->channel))) return 0;
    }
    /* Channel filter (for channel messages only) */
    uint8_t type_bit = 0;
    uint8_t s = m->status & 0xF0;
    if (s == 0x80 || s == 0x90) type_bit = MASK_NOTE;
    else if (s == 0xB0)         type_bit = MASK_CC;
    else if (s == 0xC0)         type_bit = MASK_PC;
    else if (s == 0xE0)         type_bit = MASK_PB;
    else if (m->status == 0xF0) type_bit = MASK_SX;
    else if (m->status >= 0xF8) type_bit = MASK_RT;

    return (r->msg_mask & type_bit) ? 1 : 0;
}

void router_process(port_id_t src, const midi_msg_t *msg)
{
    /* Notify application */
    if (s_rx_cb) s_rx_cb(src, msg);

    /* Forward to each destination */
    for (int dst = 0; dst < PORT_COUNT; dst++) {
        if (dst == (int)src) continue;
        if (!s_ports[dst] || !s_ports[dst]->send) continue;

        if (msg_matches(msg, &s_rules[src][dst])) {
            uint8_t buf[3];
            uint8_t len = midi_encode_msg(msg, buf, sizeof(buf));
            if (len > 0) {
                s_ports[dst]->send(buf, len);
            }
        }
    }
}

void router_poll_all(void)
{
    uint8_t buf[64];

    for (int i = 0; i < PORT_COUNT; i++) {
        if (!s_ports[i] || !s_ports[i]->poll) continue;

        uint16_t len = s_ports[i]->poll(buf, sizeof(buf));
        for (uint16_t j = 0; j < len; j++) {
            midi_msg_t msg;
            if (midi_parse(&s_parsers[i], buf[j], &msg)) {
                router_process((port_id_t)i, &msg);
            }
        }
    }
}

