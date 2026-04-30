/******************************************************************************
 * App/midi_api.h
 *
 * Convenience wrappers for sending MIDI messages.
 * Application layer calls these — never touches router/parser directly.
 *****************************************************************************/

#ifndef MIDI_API_H
#define MIDI_API_H

#include "midi_parser.h"
#include "midi_router.h"

void midi_send_note_on(port_id_t port, uint8_t ch, uint8_t note, uint8_t vel);
void midi_send_note_off(port_id_t port, uint8_t ch, uint8_t note);
void midi_send_cc(port_id_t port, uint8_t ch, uint8_t cc, uint8_t val);
void midi_send_pc(port_id_t port, uint8_t ch, uint8_t prog);
void midi_send_pitchbend(port_id_t port, uint8_t ch, uint16_t value);
void midi_send_start(port_id_t port);
void midi_send_stop(port_id_t port);
void midi_send_continue(port_id_t port);
void midi_send_all_notes_off(port_id_t port, uint8_t ch);
void midi_send_msg(port_id_t port, const midi_msg_t *msg);


#endif /* MIDI_API_H */







