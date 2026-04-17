/******************************************************************************
 * Driver/bsp_usb_midi.c
 *
 * Initializes:
 *   1. USB GPIO (PA11/PA12)
 *   2. USB clock (PLL 48MHz → USBFS)
 *   3. USBFS core via usbd_init()
 *   4. Registers MIDI class + descriptors
 *   5. Connects to bus (DP pull-up)
 *
 * Library call chain:
 *   usbd_init(&g_udev, &midi_desc, &usb_midi_cb)
 *     → usb_devcore_init()    — core reset, FIFO config
 *     → registers desc + class callbacks
 *   usbd_connect(&g_udev)
 *     → enables DP pull-up, host sees device
 *   USBFS_IRQHandler()
 *     → usbd_isr(&g_udev)    — dispatches all USB events
 *****************************************************************************/
#include "bsp_usb_midi.h"
#include "usb_midi_class.h"
#include "usb_midi_desc.h"
#include "usbd_core.h"
#include "drv_usbd_int.h"
#include "drv_usb_hw.h"
#include "bsp_system.h"

/* ---- Global USB device handle ---- */
usb_core_driver g_udev;

/* ---- Descriptor handler (passed to usbd_init) ---- */
static usb_desc midi_desc = {
    .dev_desc    = (uint8_t *)&usb_midi_dev_desc,
    .config_desc = (uint8_t *)usb_midi_config_desc,
    .strings     = usb_midi_strings,
};

/* ==================================================================
 * Hardware Configuration
 * ================================================================== */
static void usb_gpio_init(void)
{
    /* PA11 = USB_DM, PA12 = USB_DP */
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_11 | GPIO_PIN_12);

}

static void usb_clock_init(void)
{
    /* PLL output = 48MHz, USB needs 48MHz exactly.
     * RCU_CKUSB_CKPLL_DIV1: PLL/1 = 48MHz → USB */
    //rcu_usb_clock_config();
    rcu_periph_clock_enable(RCU_USBFS);
}

static void usb_irq_init(void)
{
    nvic_irq_enable(USBFS_IRQn,0U,0U);
}

/* ==================================================================
 * Public API
 * ================================================================== */
void bsp_usb_init(void)
{
    usb_gpio_init();
    usb_clock_init();
    usb_irq_init();

    /* Library initialization:
     * - Soft reset USB core
     * - Configure FIFO sizes
     * - Register descriptors and class handler
     * - Enable global interrupts
     */
    usbd_init(&g_udev, &midi_desc, &usb_midi_cb);

    /* Connect to USB bus (enable DP pull-up resistor) */
    usbd_connect(&g_udev);

}

void bsp_usb_deinit(void)
{
    usbd_disconnect(&g_udev);
    rcu_periph_clock_disable(RCU_USBFS);
}

uint8_t bsp_usb_is_configured(void)
{
    return (g_udev.dev.cur_status == USBD_CONFIGURED) ? 1U : 0U;

}

uint16_t bsp_usb_send(const uint8_t *data, uint16_t len)
{
    return usb_midi_send(&g_udev, data, len);
}

uint8_t bsp_usb_tx_ready(void)
{
    return usb_midi_tx_ready(&g_udev);
}

void bsp_usb_set_rx_callback(usb_rx_cb_t cb)
{
    usb_midi_set_rx_cb(cb);
}

/* ==================================================================
 * USB Interrupt Handler
 * ================================================================== */
void USBFS_IRQHandler(void)
{
    usbd_isr(&g_udev);
}
