/******************************************************************************
 * Middleware/usb_midi_codec.h
 *
 * USB-MIDI 1.0 Event Packet encoder/decoder.
 * Each USB-MIDI packet = 4 bytes: [CIN|Cable] [MIDI0] [MIDI1] [MIDI2]
 *****************************************************************************/
#ifndef USB_MIDI_CODEC_H
#define USB_MIDI_CODEC_H
#include "midi_parser.h"

/* Encode midi_msg_t → single USB-MIDI event packet */
typedef struct {
    uint8_t cin_cable;      /* (cable << 4) | cin */
    uint8_t midi[3];
} usb_midi_pkt_t;

/* Encode midi_msg_t → single USB-MIDI event packet */
uint8_t usb_midi_enc(const midi_msg_t *msg, uint8_t cable, usb_midi_pkt_t *pkt);

/* Decode single USB-MIDI event packet → midi_msg_t */
uint8_t usb_midi_dec(const usb_midi_pkt_t *pkt, midi_msg_t *msg);

/* Encode SysEx byte array → multiple USB-MIDI packets */
uint16_t usb_midi_enc_sysex(const uint8_t *sysex, uint16_t len, uint8_t cable, usb_midi_pkt_t *pkts, uint16_t max_pkts);

#endif /* USB_MIDI_CODEC_H*/
