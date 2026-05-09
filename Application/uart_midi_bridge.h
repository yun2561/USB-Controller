/******************************************************************************
 * App/uart_midi_bridge.h
 *
 * Bridges UART MIDI driver <-> MIDI router.
 *****************************************************************************/
#ifndef UART_MIDI_BRIDGE_H
#define UART_MIDI_BRIDGE_H

#include "common_types.h"

void uart_bridge_init(void);
void uart_bridge_drain_tx(void);


#endif /* UART_MIDI_BRIDGE_H */

