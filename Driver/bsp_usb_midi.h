/******************************************************************************
 * Driver/bsp_usb_midi.h
 *
 * GD32F350 USBFS Device — Audio Class MIDI Streaming
 *
 * Uses GD32F3x0_usbfs_library V2.5.0:
 *   - EP0: Control (64B)
 *   - EP1 OUT: Bulk OUT (Host → Device, 64B)  MIDI_OUT_EP = 0x01
 *   - EP1 IN:  Bulk IN  (Device → Host, 64B)  MIDI_IN_EP  = 0x81
 *
 * VID/PID from common_types.h (0x1209 / 0x0001)
 *****************************************************************************/


#ifndef BSP_USB_MIDI_H
#define BSP_USB_MIDI_H

#include "common_types.h"

/* ---- Lifecycle ---- */
void bsp_usb_init(void);
void bsp_usb_deinit(void);
void bsp_usb_poll(void); /* Call from main loop (handles EP0 etc.) */

/* ---- State ---- */
uint8_t bsp_usb_is_configured(void);

/* ---- Data transfer ---- */
/* Send data to Host (Bulk IN). Returns bytes actually queued (0 if busy). */
uint16_t bsp_usb_send(const uint8_t *data, uint16_t len);

/* Check if last IN transfer completed */
uint8_t  bsp_usb_tx_ready(void);

/* Receive callback: called from ISR when OUT packet arrives.
 * User must copy data immediately — buffer is reused. */
typedef void (*usb_rx_cb_t)(const uint8_t *data, uint16_t len);
void bsp_usb_set_rx_callback(usb_rx_cb_t cb);



#endif /* BSP_USB_MIDI_H */

