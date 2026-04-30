/******************************************************************************
 * App/main.c
 *
 * Initialization order:
 *   1. bsp_system_init()       — clock, SysTick, GPIO clocks, NVIC
 *   2. router_init()           — routing rules, parsers, SysEx contexts
 *   3. dfu_init()              — DFU state machine
 *   4. bsp_uart_midi_init()    — UART + DMA for DIN-5 MIDI
 *   5. bsp_usb_init()          — USBFS device + MIDI class
 *   6. uart_bridge_init()      — register UART port with router
 *   7. usb_bridge_init()       — register USB port with router
 *   8. router_set_sysex_cb()   — application SysEx dispatcher
 *   9. router_set_rx_cb()      — (optional) channel msg monitor
 *
 * Main loop:
 *   router_poll_all()          — RX: feed bytes to parser + SysEx assembler
 *   usb_bridge_drain_tx()      — TX: USB-MIDI packet encoding + send
 *   uart_bridge_drain_tx()     — TX: UART DMA send
 *****************************************************************************/
#include "gd32f3x0.h"
#include "bsp_system.h"
#include "bsp_usb_midi.h"
#include "bsp_uart_midi.h"
#include "usb_midi_bridge.h"
#include "uart_midi_bridge.h"
#include "midi_router.h"
#include "midi_api.h"
#include "dfu_engine.h"
#include "sysex_assembler.h"

/* ---- Ring buffers for USB bridge ---- */
ring_buf_t g_usb_tx;
ring_buf_t g_usb_rx;

/* ---- DFU context ---- */
static dfu_ctx_t s_dfu;

/* ---- SysEx manufacturer ID (customize per product) ---- */
#define SYSEX_MFR_ID    0x7DU       /* Non-commercial / development */
#define SYSEX_DEV_ID    0x01U
#define SYSEX_MODEL_ID  0x01U

/******************************************************************************
 * SysEx RX dispatcher
 *
 * Called by router when a complete F0..F7 message arrives on any port.
 * Protocol: F0 <mfr> <dev> <model> <cmd> [data...] F7
 *
 * Dispatches to:
 *   - DFU engine  (cmd 0x01–0x04)
 *   - Config      (future)
 *   - Preset      (future)
 *****************************************************************************/
static void on_sysex_rx(port_id_t port, const uint8_t *data, uint16_t len)
{
    (void)port;

    /* Minimum: F0 mfr dev model cmd F7 = 6 bytes */
    if (len < 6) return;

    /* Check our manufacturer / device / model IDs */
    if (data[1] != SYSEX_MFR_ID ||
        data[2] != SYSEX_DEV_ID ||
        data[3] != SYSEX_MODEL_ID) {
        return;     /* Not addressed to us — ignore (already forwarded by router) */
    }

    uint8_t cmd = data[4];

    switch (cmd)
    {
        /* ---- DFU commands ---- */
        case DFU_CMD_START:
        case DFU_CMD_DATA:
        case DFU_CMD_VERIFY:
        case DFU_CMD_REBOOT:
            dfu_process_sysex(&s_dfu, data, len);
            break;

        /* ---- Future: configuration, preset, identity, etc. ---- */
        /*
        case CMD_CONFIG_DUMP:
        case CMD_CONFIG_LOAD:
        case CMD_IDENTITY_REQ:
            break;
        */

        default:
            break;
    }
}

/******************************************************************************
 * Optional: channel/RT message monitor callback
 * Useful for LED feedback, activity indicator, etc.
 *****************************************************************************/
/*
static void on_midi_rx(port_id_t port, const midi_msg_t *msg)
{
    (void)port;
    (void)msg;
    // ui_manager_midi_activity(port, msg);
}
*/

/******************************************************************************
 * DFU progress / complete callbacks
 *****************************************************************************/
static void on_dfu_progress(uint8_t percent)
{
    (void)percent;
    /* TODO: update LED / display */
}

static void on_dfu_complete(uint8_t success)
{
    (void)success;
    /* TODO: LED feedback, then reboot if success */
}

/******************************************************************************
 * main
 *****************************************************************************/
int main(void)
{
    /* ---- 1. Hardware ---- */
    bsp_system_init();

    /* ---- 2. Middleware ---- */
    router_init();
    dfu_init(&s_dfu);
    dfu_set_callbacks(on_dfu_progress, on_dfu_complete);

    /* ---- 3. Drivers ---- */
    bsp_uart_midi_init();
    bsp_usb_init();

    /* ---- 4. Bridges (register ports with router) ---- */
    uart_bridge_init();
    usb_bridge_init();

    /* ---- 5. Application callbacks ---- */
    router_set_sysex_cb(on_sysex_rx);
    /* router_set_rx_cb(on_midi_rx); */

    /* ---- Main loop ---- */
    while (1)
    {
        router_poll_all();              /* RX: parse + assemble SysEx */
        usb_bridge_drain_tx();          /* TX: USB-MIDI packets out   */
        uart_bridge_drain_tx();         /* TX: UART DMA out           */
    }

    return 0;
}