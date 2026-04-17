#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "gd32f3x0.h"
#include <stdint.h>

#include <string.h>
#include <stdlib.h>

/* ---- Flash Memory Map ---- */
#define FLASH_BASE_ADDR         0x08000000U
#define BOOTLOADER_ADDR         0x08000000U
#define APP_ADDR                0x08002000U
#define CONFIG_ADDR             0x0800E000U
#define DFU_STAGING_ADDR        0x0800F000U
#define FMC_PAGE_SIZE           1024U

/* ---- DFU Magic ---- */
#define DFU_MAGIC_UPGRADE       0xDEADBEEFU
#define DFU_MAGIC_NORMAL        0x00000000U

/* ---- MIDI ---- */
#define MIDI_UART               USART0
#define MIDI_UART_CLK           RCU_USART0
#define MIDI_UART_IRQn          USART0_IRQn
#define MIDI_UART_BAUD          31250
#define MIDI_TX_PORT            GPIOA
#define MIDI_TX_PIN             GPIO_PIN_9
#define MIDI_TX_AF              GPIO_AF_1
#define MIDI_RX_PORT            GPIOA
#define MIDI_RX_PIN             GPIO_PIN_10
#define MIDI_RX_AF              GPIO_AF_1
#define MIDI_TX_DMA_CH          DMA_CH1         /* USART0_TX */
#define MIDI_TX_DMA_IRQn        DMA_Channel1_2_IRQn
#define MIDI_RX_DMA_CH          DMA_CH2         /* USART0_TX */
#define MIDI_RX_DMA_IRQn        DMA_Channel3_4_IRQn

/* ---- Ring buffer (power of 2) ---- */
#define RING_SIZE               256



/* ---- System ---- */
#define SYSTICK_FREQ_HZ         1000U

/* ---- USB IDs ---- */
#define USBD_VID                0x1209      /* pid.codes open-source VID */
#define USBD_PID                0x0001

/* ---- USB Endpoints (USBFS Device) ---- */
#define MIDI_IN_EP              0x81        /* Bulk IN  EP1 */
#define MIDI_OUT_EP             0x01        /* Bulk OUT EP1 */
#define MIDI_MAX_PACKET         64

/* ---- Product Info ---- */
#define PRODUCT_NAME            "Smart Widget"
#define FW_VERSION_MAJOR        1
#define FW_VERSION_MINOR        0
#define FW_VERSION_PATCH        0

/* ---- Generic Callbacks ---- */
typedef void (*callback_void_t)(void);
typedef void (*callback_u8_t)(uint8_t);

#endif /* COMMON_TYPES_H */
