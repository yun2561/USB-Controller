/******************************************************************************
 * Middleware/midi_parser.c
 *****************************************************************************/

#include "midi_parser.h"

#include <string.h>

uint8_t midi_data_len(uint8_t s)
{
    if (s >= 0x80 && s <= 0xBF) return 2;   /* Note Off/On, PolyAT, CC */
    if (s >= 0xC0 && s <= 0xDF) return 1;   /* PC, Channel AT */
    if (s >= 0xE0 && s <= 0xEF) return 2;   /* Pitch Bend */
    switch (s) {
    case 0xF1: case 0xF3: return 1;         /* MTC QF, Song Select */
    case 0xF2: return 2;                     /* Song Position */
    default: return 0;
    }
}

void midi_parser_init(midi_parser_t *p)
{
    memset(p,0,sizeof(*p));
}

uint8_t midi_parse(midi_parser_t *p, uint8_t byte, midi_msg_t *out)
{
    /* Real-time messages (0xF8-0xFF) pass through immediately */
    if (byte >= 0xF8 )
    {
        out->status     = byte;
        out->channel    = 0;
        out->data[0]    = out->data[1] = 0;
        out->length     = 1;
        return 1;
    }

    /* SysEx start */
    if (byte == MIDI_SYSEX)
    {
        p->state        = PS_SYSEX;
        p->msg.status   = PS_SYSEX;
        p->running      = 0;
        p->msg.length   = 0;
        return 0;
        
    }

    /* SysEx end — handled by sysex_assembler, not parser */
    if (byte == 0xF7)
    {
        p->state        = PS_IDLE;
        return 0;
    }

    /* Status byte */
    if (byte & 0x80)
    {
        p->msg.status   = byte;
        p->msg.channel  = byte & 0x0F;
        p->running      = byte;
        uint8_t expected = midi_data_len(byte);
        if(expected == 0)
        {
            /* No data bytes — complete message */
            *out        = p->msg;
            out->length = 1;
            p->state    = PS_IDLE;
        }
        p->state        = PS_D1;
        return 0;
    }

    /* Data byte */
    switch (p->state)
    {
        case PS_IDLE:
        /* Running status: reuse last status */         
            if (p->running){
                p->msg.status   = p->running;
                p->msg.channel  = p->running & 0x0F;
                p->msg.data[0]  = byte;
                if (midi_data_len(p->running) <= 1){
                    *out        = p->msg;
                    out->length = 2;
                    return 1;
                }
                p->state        = PS_D2;
            }
            return 0;

        case PS_D1:
            p->msg.data[0]  = byte;
            *out            = p->msg;
            out->length     = 3;
            p->state        = PS_IDLE;
            return 1;
        
        case PS_D2:
            p->msg.data[1]  = byte;
            *out            = p->msg;
            out->length     = 3;
            p->state        = PS_IDLE;
            return 1;
        
        case PS_SYSEX:
            /* SysEx data — accumulated by sysex_assembler */
            return 0;

        
        default:
            p->state = PS_IDLE;
            return 0;
    }
}

uint8_t midi_encode_msg(const midi_msg_t *m, uint8_t *buf, uint8_t max)
{
    if (max < 1) return 0;
    buf[0] = m->status;
    uint8_t dlen = midi_data_len(m->status);
    if (dlen >= 1 && max >= 2) buf[1] = m->data[0];
    if (dlen >= 2 && max >= 3) buf[2] = m->data[1];
    return 1 + dlen;
}