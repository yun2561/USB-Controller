
/******************************************************************************
 * App/usb_midi_bridge.h
 *
 * Bridges USB-MIDI driver ↔ Middleware ring buffer ↔ MIDI router.
 *
 * RX: USB ISR → rb_push(g_usb_rx) → poll decodes CIN → router
 * TX: router → rb_push(g_usb_tx) → drain re-encodes → bsp_usb_send
 *****************************************************************************/
#ifndef USB_MIDI_BRIDGE_H
#define USB_MIDI_BRIDGE_H

#include "common_types.h"

void usb_bridge_init(void);         /* Call once after bsp_usb_init() */
void usb_bridge_drain_tx(void);     /* Call from main loop after router_poll_all() */


#endif /* USB_MIDI_BRIDGE_H */
