/******************************************************************************
 * Middleware/midi_parser.h
 *****************************************************************************/
#ifndef MIDI_PARSER_H
#define MIDI_PARSER_H

#include "common_types.h"

/* ---- Status bytes ---- */
#define MIDI_NOTE_OFF           0x80
#define MIDI_NOTE_ON            0x90
#define MIDI_POLY_AT            0xA0
#define MIDI_CC                 0xB0
#define MIDI_PC                 0xC0
#define MIDI_CHAN_AT            0xD0
#define MIDI_PITCH_BEND         0xE0
#define MIDI_SYSEX              0xF0
#define MIDI_MTC                0xF1
#define MIDI_SPP                0xF2
#define MIDI_SONG_SEL           0xF3
#define MIDI_TUNE_REQ           0xF6
#define MIDI_EOX                0xF7
#define MIDI_CLOCK              0xF8
#define MIDI_START              0xFA
#define MIDI_CONTINUE           0xFB
#define MIDI_STOP               0xFC
#define MIDI_ACTIVE_SENSE       0xFE
#define MIDI_RESET              0xFF

/* MIDI message struct */
typedef struct {
    uint8_t status;         /* Full status byte (0x80-0xFF) */
    uint8_t channel;        /* 0-15 for channel messages    */
    uint8_t data[2];        /* Data bytes                   */
    uint8_t length;         /* Total bytes (1-3)            */
} midi_msg_t;

/* Parser state machine */
typedef enum {
    PS_IDLE, PS_STATUS, PS_D1, PS_D2, PS_SYSEX
} parser_state_t;

typedef struct {
    parser_state_t state;
    uint8_t        running;     /* Running status byte */
    midi_msg_t     msg;
} midi_parser_t;

void    midi_parser_init(midi_parser_t *p);
uint8_t midi_parse(midi_parser_t *p, uint8_t byte, midi_msg_t *out);
uint8_t midi_encode_msg(const midi_msg_t *m, uint8_t *buf, uint8_t max);
uint8_t midi_data_len(uint8_t status);

#endif /* MIDI_PARSER_H */
