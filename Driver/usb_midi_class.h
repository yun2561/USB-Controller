/******************************************************************************
 * Driver/usb_midi_class.h
 *
 * USB-MIDI Class Handler for GD32F3x0 USBFS Library V2.6.0
 *
 * Library structures:
 *   usb_core_driver  (typedef'd as usb_dev)
 *     .bp             — basic parameters
 *     .regs           — register base
 *     .dev            — usb_perp_dev
 *       .cur_status   — USBD_DEFAULT / USBD_CONFIGURED / ...
 *       .transc_out[] — OUT endpoint transaction info
 *       .class_core   — pointer to usb_class_core
 *
 * Required library files:
 *   drv_usb_core.c, drv_usb_dev.c, drv_usbd_int.c
 *   usbd_core.c, usbd_enum.c, usbd_transc.c, usbd_pwr.c
 *****************************************************************************/
#ifndef USB_MIDI_CLASS_H
#define USB_MIDI_CLASS_H

#include "usbd_core.h"      /* usb_core_driver, usbd_ep_setup, etc. */
#include "common_types.h"

/* MIDI class data — lives as static, referenced via class_core->data */
typedef struct {
    uint8_t          rx_buf[MIDI_MAX_PACKET];
    uint16_t         rx_len;
    uint8_t          tx_buf[MIDI_MAX_PACKET];
    volatile uint8_t tx_busy;
    void           (*rx_cb)(const uint8_t *data, uint16_t len);
} midi_class_data_t;

/* Class core instance — register this with usbd_init() */
extern usb_class_core usb_midi_cb;

/* ---- Application API ---- */
void     usb_midi_set_rx_cb(void (*cb)(const uint8_t *data, uint16_t len));
uint16_t usb_midi_send(usb_core_driver *udev, const uint8_t *data, uint16_t len);
uint8_t  usb_midi_tx_ready(usb_core_driver *udev);


#endif /* USB_MIDI_CLASS_H */
