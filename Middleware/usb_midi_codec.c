/******************************************************************************
 * Middleware/usb_midi_codec.c
 *****************************************************************************/
#include "usb_midi_codec.h"


static uint8_t status_to_cin(uint8_t st)
{
    if (st < 0x80) return 0;
    if (st < 0xF0) return (st >> 4); /* Channel msg: CIN = high nibble */
    switch (st)
    {
        case 0xF1: case 0xF3: return 0x02; /* 2-byte system common */
        case 0xF2:            return 0x03; /* 3-byte system common */
        case 0xF6:            return 0x05; /* 1-byte system common */
        default:              return 0x0F; /* Single byte */
    }
}

uint8_t usb_midi_enc(const midi_msg_t *msg, uint8_t cable, usb_midi_pkt_t *pkt)
{
    uint8_t cin = status_to_cin(msg->status);
    pkt->cin_cable = ((cable & 0x0F) << 4) | (cin & 0x0F);
    pkt->midi[0]   = msg->status;
    pkt->midi[1]   = msg->data[0];
    pkt->midi[2]   = msg->data[1];
    return 1;
}


uint8_t usb_midi_dec(const usb_midi_pkt_t *pkt, midi_msg_t *msg)
{
    uint8_t cin  = pkt->cin_cable & 0x0F;
    msg->data[0] = pkt->midi[1];
    msg->data[1] = pkt->midi[2];
    msg->channel = (msg->status < 0xF0) ? (msg->status & 0x0F) : 0;
    switch (cin)
    {
        case 0x05: case 0x0F:               msg->length = 1; break; /* 1-byte */
        case 0x02: case 0x0C: case 0x0D:    msg->length = 2; break; /* 2-byte */
        default:                            msg->length = 3; break; /* 3-byte */
    }
    return 1;
}


uint16_t usb_midi_enc_sysex(const uint8_t *sysex, uint16_t len, uint8_t cable, usb_midi_pkt_t *pkts, uint16_t max_pkts)
{
    uint16_t pi = 0, i = 0;
    uint8_t cn = (cable & 0x0F) << 4;

    while (i < len && pi < max_pkts){
        uint16_t rem = len - i;

        if (rem >= 3){
            /* Last 3 bytes ending with F7? → CIN 0x07 (SysEx end 3) */
            uint8_t cin         = (rem == 3 && sysex[i+2] == 0xF7) ? 0x07 : 0x04;
            pkts[pi].cin_cable  = cn | cin;
            pkts[pi].midi[0]    = sysex[i];
            pkts[pi].midi[1]    = sysex[i+1];
            pkts[pi].midi[2]    = sysex[i+2];
        }
        else if (rem == 2){
            pkts[pi].cin_cable  = cn | 0x06; /* SysEx end 2 bytes */
            pkts[pi].midi[0]    = sysex[i];
            pkts[pi].midi[1]    = sysex[i+1];
            pkts[pi].midi[2]    = 0;

        }
        else {
            pkts[pi].cin_cable  = cn | 0x05; /* SysEx end 1 byte */
            pkts[pi].midi[0] = sysex[i];
            pkts[pi].midi[1] = 0;
            pkts[pi].midi[2] = 0;
            i += 1;
        }
        pi++;
    }
    return pi;
}


