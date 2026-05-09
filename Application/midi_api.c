/******************************************************************************
 * App/midi_api.c
 *****************************************************************************/
#include "midi_api.h"


/* Route message through internal port so router applies rules */
static void send_via_router(const midi_msg_t *msg)
{
    router_process(PORT_INT,msg);
}


void midi_send_note_on(port_id_t port, uint8_t ch, uint8_t note, uint8_t vel)
{
    (void)port;
    midi_msg_t m = { .status = 0x90U | (ch & 0x0F),
                     .channel = ch & 0x0F,
                     .data = { note & 0x7F, vel & 0x7F },
                     .length = 3 };
    send_via_router(&m);
}

void midi_send_note_off(port_id_t port, uint8_t ch, uint8_t note)
{
    (void)port;
    midi_msg_t m = { .status = 0x80U | (ch & 0x0F),
                     .channel = ch & 0x0F,
                     .data = { note & 0x7F, 0 },
                     .length = 3 };
    send_via_router(&m);
}

void midi_send_cc(port_id_t port, uint8_t ch, uint8_t cc, uint8_t val)
{
    (void)port;
    midi_msg_t m = { .status = 0xB0U | (ch & 0x0F),
                     .channel = ch & 0x0F,
                     .data = { cc & 0x7F, val & 0x7F },
                     .length = 3 };
    send_via_router(&m);
}

void midi_send_pc(port_id_t port, uint8_t ch, uint8_t prog)
{
    (void)port;
    midi_msg_t m = { .status = 0xC0U | (ch & 0x0F),
                     .channel = ch & 0x0F,
                     .data = { prog & 0x7F, 0 },
                     .length = 2 };
    send_via_router(&m);
}

void midi_send_pitchbend(port_id_t port, uint8_t ch, uint16_t value)
{
    (void)port;
    uint16_t raw = (uint16_t)(value + 8192);
    midi_msg_t m = { .status = 0xE0U | (ch & 0x0F),
                     .channel = ch & 0x0F,
                     .data = { raw & 0x7F, (raw >> 7) & 0x7F },
                     .length = 3 };
    send_via_router(&m);
}

void midi_send_start(port_id_t port)
{
    (void)port;
    midi_msg_t m = { .status = 0xFA, .length = 1 };
    send_via_router(&m);
}

void midi_send_stop(port_id_t port)
{
    (void)port;
    midi_msg_t m = { .status = 0xFC, .length = 1 };
    send_via_router(&m);
}

void midi_send_continue(port_id_t port)
{
    (void)port;
    midi_msg_t m = { .status = 0xFB, .length = 1 };
    send_via_router(&m);
}

void midi_send_all_notes_off(port_id_t port, uint8_t ch)
{
    midi_send_cc(port, ch, 123, 0);
}

void midi_send_msg(port_id_t port, const midi_msg_t *msg)
{
    (void)port;
    send_via_router(msg);
}
