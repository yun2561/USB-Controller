/******************************************************************************
 * Driver/usbd_conf.h
 *
 * GD32F3x0 USBFS Device Library V2.6.0 Configuration
 *
 * This file is #included by the library internals.
 * Place it in your project include path.
 *****************************************************************************/
#ifndef USBD_CONF_H
#define USBD_CONF_H

#include "gd32f3x0.h"

/* ---- Device mode ---- */
#define USE_DEVICE_MODE

/* ---- Configuration ---- */
#define USBD_CFG_MAX_NUM            1U
#define USBD_ITF_MAX_NUM            2U      /* Interface 0: AC, Interface 1: MS */

/* ---- Endpoints ---- */
/* EP_COUNT = highest EP number + 1 (EP0 + EP1 = 2) */
#define EP_COUNT                    2U
#define USB_FS_EP0_MAX_LEN          64U

/* ---- Strings ---- */
#define USB_STRING_COUNT            4U
#define USB_STR_DESC_MAX_SIZE       64U

/* ---- RX/TX FIFO sizes (in 32-bit words) ----
 * Total USBFS FIFO = 320 words (1.25KB) on GD32F350
 * Allocation:
 *   RX  = 128 words (512B) — shared for all OUT EPs
 *   TX0 = 64  words (256B) — EP0 IN (control)
 *   TX1 = 64  words (256B) — EP1 IN (MIDI Bulk)
 *   Sum = 256 words (fits in 320 budget)
 */
//#define RX_FIFO_FS_SIZE             128U
//#define TX0_FIFO_FS_SIZE            64U
//#define TX1_FIFO_FS_SIZE            64U
//#define TX2_FIFO_FS_SIZE            0U
//#define TX3_FIFO_FS_SIZE            0U

/* ---- Power ---- */
//#define USBD_SELF_POWERED           0U      /* Bus-powered */

#endif /* USBD_CONF_H */

