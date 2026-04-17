/******************************************************************************
 * Driver/usb_midi_desc.c
 *
 * Descriptor tree:
 *
 * Device Descriptor (18B)
 * └─ Configuration (101B total)
 *    ├─ Interface 0: Audio Control       (9B)
 *    │  └─ AC Header                     (9B)
 *    └─ Interface 1: MIDI Streaming      (9B)
 *       ├─ MS Header                     (7B)
 *       ├─ IN Jack Embedded   ID=1       (6B)
 *       ├─ IN Jack External   ID=2       (6B)
 *       ├─ OUT Jack Embedded  ID=3 ←J2   (9B)
 *       ├─ OUT Jack External  ID=4 ←J1   (9B)
 *       ├─ Bulk OUT EP 0x01              (9B)
 *       ├─ MS Bulk OUT CS-EP  →J1        (5B)
 *       ├─ Bulk IN  EP 0x81              (9B)
 *       └─ MS Bulk IN  CS-EP →J3         (5B)
 *****************************************************************************/
#include "usb_midi_desc.h"
#include <string.h>


/* ==================================================================
 * Device Descriptor
 * ================================================================== */
const usb_desc_dev usb_midi_dev_desc = {
    .header = {
        .bLength         = USB_DEV_DESC_LEN,
        .bDescriptorType = USB_DESCTYPE_DEV
    },
    .bcdUSB                = 0x0200U,        /* USB 2.0 */
    .bDeviceClass          = 0x00U,          /* Per-interface */
    .bDeviceSubClass       = 0x00U,
    .bDeviceProtocol       = 0x00U,
    .bMaxPacketSize0       = USB_FS_EP0_MAX_LEN,  /* 64 */
    .idVendor              = USBD_VID,       /* 0x1209 */
    .idProduct             = USBD_PID,       /* 0x0001 */
    .bcdDevice             = 0x0100U,        /* Device version 1.00 */
    .iManufacturer         = 1U,
    .iProduct              = 2U,
    .iSerialNumber         = 3U,
    .bNumberConfigurations = 1U
};


/* ==================================================================
 * Configuration + All Class/EP Descriptors (raw byte array)
 * Sent as single blob on GET_DESCRIPTOR(Configuration)
 * ================================================================== */
const uint8_t usb_midi_config_desc[USB_MIDI_CONFIG_DESC_LEN] = {

    /* ---- Configuration (9B) ---- */
    0x09, USB_DESCTYPE_CONFIG,
    USB_MIDI_CONFIG_DESC_LEN & 0xFF,
    USB_MIDI_CONFIG_DESC_LEN >> 8,
    0x02,                           /* bNumInterfaces: 2 (AC + MS)    */
    0x01,                           /* bConfigurationValue             */
    0x00,                           /* iConfiguration                  */
    0x80,                           /* bmAttributes: Bus-powered       */
    0x32,                           /* bMaxPower: 100mA                */

    /* ---- Interface 0: Audio Control (9B) ---- */
    0x09, USB_DESCTYPE_ITF,
    MIDI_AC_ITF,                    /* bInterfaceNumber: 0             */
    0x00, 0x00,                     /* bAlternateSetting, bNumEndpoints*/
    0x01, 0x01, 0x00, 0x00,        /* Audio, AC, Protocol, iInterface */

    /* ---- AC Header CS (9B) ---- */
    0x09, USB_CS_INTERFACE, AC_HEADER,
    0x00, 0x01,                     /* bcdADC: 1.00                    */
    0x09, 0x00,                     /* wTotalLength: 9                 */
    0x01,                           /* bInCollection: 1                */
    MIDI_MS_ITF,                    /* baInterfaceNr(1): 1             */

    /* ---- Interface 1: MIDI Streaming (9B) ---- */
    0x09, USB_DESCTYPE_ITF,
    MIDI_MS_ITF,                    /* bInterfaceNumber: 1             */
    0x00,                           /* bAlternateSetting               */
    0x02,                           /* bNumEndpoints: 2 (Bulk IN+OUT) */
    0x01, 0x03, 0x00, 0x00,        /* Audio, MS, Protocol, iInterface */

    /* ---- MS Header CS (7B) ---- */
    0x07, USB_CS_INTERFACE, MS_HEADER,
    0x00, 0x01,                     /* bcdMSC: 1.00                    */
    0x41, 0x00,                     /* wTotalLength: 65                */

    /* ---- MIDI IN Jack: Embedded, ID=1 (6B) ----
     * Receives data FROM Host (via Bulk OUT EP) */
    0x06, USB_CS_INTERFACE, MIDI_IN_JACK,
    JACK_EMBEDDED, JACK_EMB_IN, 0x00,

    /* ---- MIDI IN Jack: External, ID=2 (6B) ----
     * Represents physical DIN-5 MIDI IN connector */
    0x06, USB_CS_INTERFACE, MIDI_IN_JACK,
    JACK_EXTERNAL, JACK_EXT_IN, 0x00,

    /* ---- MIDI OUT Jack: Embedded, ID=3 (9B) ----
     * Sends data TO Host (via Bulk IN EP)
     * Source: External IN Jack (ID=2, physical MIDI IN) */
    0x09, USB_CS_INTERFACE, MIDI_OUT_JACK,
    JACK_EMBEDDED, JACK_EMB_OUT,
    0x01,                           /* bNrInputPins                    */
    JACK_EXT_IN, 0x01, 0x00,       /* baSourceID, baSourcePin, iJack  */

    /* ---- MIDI OUT Jack: External, ID=4 (9B) ----
     * Represents physical DIN-5 MIDI OUT connector
     * Source: Embedded IN Jack (ID=1, data from Host) */
    0x09, USB_CS_INTERFACE, MIDI_OUT_JACK,
    JACK_EXTERNAL, JACK_EXT_OUT,
    0x01,
    JACK_EMB_IN, 0x01, 0x00,

    /* ---- Bulk OUT Endpoint: Host → Device (9B) ---- */
    0x09, USB_DESCTYPE_EP,
    MIDI_OUT_EP,                    /* 0x01                            */
    USB_EP_ATTR_BULK,
    MIDI_MAX_PACKET & 0xFF, MIDI_MAX_PACKET >> 8,
    0x00, 0x00, 0x00,

    /* ---- MS Bulk OUT CS-EP (5B) ---- */
    0x05, USB_CS_ENDPOINT, MS_GENERAL,
    0x01, JACK_EMB_IN,             /* Assoc Jack 1 (Embedded IN)      */

    /* ---- Bulk IN Endpoint: Device → Host (9B) ---- */
    0x09, USB_DESCTYPE_EP,
    MIDI_IN_EP,                     /* 0x81                            */
    USB_EP_ATTR_BULK,
    MIDI_MAX_PACKET & 0xFF, MIDI_MAX_PACKET >> 8,
    0x00, 0x00, 0x00,

    /* ---- MS Bulk IN CS-EP (5B) ---- */
    0x05, USB_CS_ENDPOINT, MS_GENERAL,
    0x01, JACK_EMB_OUT,            /* Assoc Jack 3 (Embedded OUT)     */
};


/* ==================================================================
 * String Descriptors
 * ================================================================== */

/* String 0: Language ID */
static usb_desc_str s_langid = {
    .header = { .bLength = 4U, .bDescriptorType = USB_DESCTYPE_STR },
    .unicode_string = { 0x0409U }   /* English (US) */
};

/* String 1: Manufacturer — "MusicTrib" */
static usb_desc_str s_mfr = {
    .header = { .bLength = 2U + 9U * 2U, .bDescriptorType = USB_DESCTYPE_STR },
    .unicode_string = { 'M','u','s','i','c','T','r','i','b' }
};

/* String 2: Product — "Smart Widget" */
static usb_desc_str s_prod = {
    .header = { .bLength = 2U + 12U * 2U, .bDescriptorType = USB_DESCTYPE_STR },
    .unicode_string = { 'S','m','a','r','t',' ','W','i','d','g','e','t' }
};

/* String 3: Serial — "00000001" */
static usb_desc_str s_serial = {
    .header = { .bLength = 2U + 8U * 2U, .bDescriptorType = USB_DESCTYPE_STR },
    .unicode_string = { '0','0','0','0','0','0','0','1' }
};

/* String table (indexed by iManufacturer/iProduct/iSerialNumber) */
void *const usb_midi_strings[USB_MIDI_STR_COUNT] = {
    [0] = (void *)&s_langid,
    [1] = (void *)&s_mfr,
    [2] = (void *)&s_prod,
    [3] = (void *)&s_serial,
};
