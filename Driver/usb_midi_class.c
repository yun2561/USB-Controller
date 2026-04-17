
 /* Driver/usb_midi_class.c
 *
 * Implements usb_class_core callbacks:
 *   .init      — SET_CONFIGURATION received
 *   .deinit    — USB reset / deconfiguration
 *   .req_proc  — Class-specific EP0 requests
 *   .data_in   — EP1 IN transfer complete (Device → Host)
 *   .data_out  — EP1 OUT transfer complete (Host → Device)
 *****************************************************************************/
#include "usb_midi_class.h"
#include "usb_midi_desc.h"

#include <string.h>

/* Static class data */
static midi_class_data_t s_midi_data;

/* ---- Forward declarations ---- */
static uint8_t midi_init   (usb_core_driver *udev, uint8_t cfgidx);
static uint8_t midi_deinit (usb_core_driver *udev, uint8_t cfgidx);
static uint8_t midi_req    (usb_core_driver *udev, usb_req *req);
static uint8_t midi_in     (usb_core_driver *udev, uint8_t ep_num);
static uint8_t midi_out    (usb_core_driver *udev, uint8_t ep_num);

/* ---- Class core structure ---- */
usb_class_core usb_midi_cb = {
    .command   = 0xFFU,
    .alter_set = 0U,

    .init      = midi_init,
    .deinit    = midi_deinit,
    .req_proc  = midi_req,
    .data_in   = midi_in,
    .data_out  = midi_out,
};


/* ==================================================================
 * Init — called on SET_CONFIGURATION(1)
 * ================================================================== */
static uint8_t midi_init(usb_core_driver *udev, uint8_t cfgidx)
{
    (void)cfgidx;

    memset(&s_midi_data, 0, sizeof(s_midi_data));

    /* ---- Open Bulk OUT endpoint (Host → Device) ---- */
    static const usb_desc_ep ep_out = {
        .header = {
            .bLength         = sizeof(usb_desc_ep),
            .bDescriptorType = USB_DESCTYPE_EP
        },
        .bEndpointAddress  = MIDI_OUT_EP,        /* 0x01 */
        .bmAttributes      = USB_EP_ATTR_BULK,
        .wMaxPacketSize    = MIDI_MAX_PACKET,     /* 64 */
        .bInterval         = 0U
    };
    usbd_ep_setup(udev, &ep_out);

    /* ---- Open Bulk IN endpoint (Device → Host) ---- */
    static const usb_desc_ep ep_in = {
        .header = {
            .bLength         = sizeof(usb_desc_ep),
            .bDescriptorType = USB_DESCTYPE_EP
        },
        .bEndpointAddress  = MIDI_IN_EP,          /* 0x81 */
        .bmAttributes      = USB_EP_ATTR_BULK,
        .wMaxPacketSize    = MIDI_MAX_PACKET,
        .bInterval         = 0U
    };
    usbd_ep_setup(udev, &ep_in);

    /* ---- Prepare OUT EP to receive first packet ---- */
    usbd_ep_recev(udev, MIDI_OUT_EP, s_midi_data.rx_buf, MIDI_MAX_PACKET);

    s_midi_data.tx_busy = 0U;

    

    return USBD_OK;
}

/* ==================================================================
 * Deinit — called on USB reset or deconfiguration
 * ================================================================== */
static uint8_t midi_deinit(usb_core_driver *udev, uint8_t cfgidx)
{
    (void)cfgidx;

    usbd_ep_clear(udev, MIDI_OUT_EP);
    usbd_ep_clear(udev, MIDI_IN_EP);

    s_midi_data.tx_busy = 0U;

    

    return USBD_OK;
}

/* ==================================================================
 * Class Request — MIDI has no mandatory class-specific requests.
 * Just ACK anything the host sends.
 * ================================================================== */
static uint8_t midi_req(usb_core_driver *udev, usb_req *req)
{
    (void)req;

    /* Send ZLP ACK on EP0 IN */
    usbd_ep_send(udev, 0x80U, NULL, 0U);

    return USBD_OK;
}

/* ==================================================================
 * Data IN — EP1 IN transfer complete (we sent data to Host)
 * ================================================================== */
static uint8_t midi_in(usb_core_driver *udev, uint8_t ep_num)
{
    (void)udev;

    if (ep_num == (MIDI_IN_EP & 0x7FU)) {
        s_midi_data.tx_busy = 0U;
    }

    return USBD_OK;
}

/* ==================================================================
 * Data OUT — EP1 OUT transfer complete (Host sent data to us)
 * ================================================================== */
static uint8_t midi_out(usb_core_driver *udev, uint8_t ep_num)
{
    if (ep_num == MIDI_OUT_EP)
    {
       /* Get actual received byte count */
       s_midi_data.rx_len = usbd_rxcount_get(udev, MIDI_OUT_EP);

       /* Notify application (ISR context!) */
       if (s_midi_data.rx_cb && s_midi_data.rx_len > 0U)
       {
            s_midi_data.rx_cb(s_midi_data.rx_buf, s_midi_data.rx_len);
       }
       /* Re-arm OUT endpoint for next packet */
       usbd_ep_recev(udev, MIDI_OUT_EP, s_midi_data.rx_buf, MIDI_MAX_PACKET);
    }
    return USBD_OK;
}

/* ==================================================================
 * Public API
 * ================================================================== */
void usb_midi_set_rx_cb(void (*cb)(const uint8_t *data, uint16_t len))
{
    s_midi_data.rx_cb = cb;
}

uint16_t usb_midi_send(usb_core_driver *udev, const uint8_t *data, uint16_t len)
{
    if (udev->dev.cur_status != USBD_CONFIGURED) return 0U;
    if (s_midi_data.tx_busy) return 0U;
    if (len > MIDI_MAX_PACKET) len = MIDI_MAX_PACKET;

    memcpy(s_midi_data.tx_buf, data, len);
    s_midi_data.tx_busy = 1U;

    usbd_ep_send(udev, MIDI_IN_EP, s_midi_data.tx_buf, (uint32_t)len);

    return len;
}

uint8_t usb_midi_tx_ready(usb_core_driver *udev)
{
    if (udev->dev.cur_status != USBD_CONFIGURED) return 0U;
    return s_midi_data.tx_busy ? 0U : 1U;
}
