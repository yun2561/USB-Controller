
/******************************************************************************
 * Driver/usb_midi_desc.h
 *
 * USB-MIDI Descriptors for GD32F3x0 USBFS Library V2.6.0
 *
 * Library types used:
 *   usb_desc_dev    — Device descriptor struct (from usb_ch9_std.h)
 *   usb_desc_ep     — Endpoint descriptor struct
 *   usb_desc_str    — String descriptor struct
 *   usb_desc        — Descriptor handler { dev_desc, config_desc, strings }
 *****************************************************************************/
#ifndef USB_MIDI_DESC_H
#define USB_MIDI_DESC_H

#include "usbd_core.h"       /* Pulls in drv_usb_dev.h → usb_ch9_std.h */
#include "common_types.h"

/* ---- Descriptor sizes ---- */
#define USB_MIDI_CONFIG_DESC_LEN    101U
#define USB_MIDI_STR_COUNT          4U

/* ---- Interface numbers ---- */
#define MIDI_AC_ITF                 0U
#define MIDI_MS_ITF                 1U

/* ---- Jack IDs ---- */
#define JACK_EMB_IN                 1U
#define JACK_EXT_IN                 2U
#define JACK_EMB_OUT                3U
#define JACK_EXT_OUT                4U

/* ---- CS descriptor types ---- */
#define USB_CS_INTERFACE            0x24U
#define USB_CS_ENDPOINT             0x25U

/* ---- Audio/MIDI subtypes ---- */
#define AC_HEADER                   0x01U
#define MS_HEADER                   0x01U
#define MIDI_IN_JACK                0x02U
#define MIDI_OUT_JACK               0x03U
#define MS_GENERAL                  0x01U
#define JACK_EMBEDDED               0x01U
#define JACK_EXTERNAL               0x02U

/* ---- Extern declarations ---- */
extern const usb_desc_dev  usb_midi_dev_desc;
extern const uint8_t       usb_midi_config_desc[USB_MIDI_CONFIG_DESC_LEN];
extern void *const         usb_midi_strings[USB_MIDI_STR_COUNT];

#endif /* USB_MIDI_DESC_H */
