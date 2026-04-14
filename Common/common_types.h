#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>
//#include <string.h>
//#include <stdlib.h>

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
#define MIDI_UART_BAUDRATE      31250U
#define MIDI_USB_EP_SIZE        64U
#define MIDI_CH_COUNT           16U
#define MIDI_NOTE_COUNT         128U

/* ---- System ---- */
#define SYSTICK_FREQ_HZ         1000U

/* ---- Product Info ---- */
#define PRODUCT_NAME            "Smart Widget"
#define FW_VERSION_MAJOR        1
#define FW_VERSION_MINOR        0
#define FW_VERSION_PATCH        0

/* ---- Generic Callbacks ---- */
typedef void (*callback_void_t)(void);
typedef void (*callback_u8_t)(uint8_t);

#endif /* COMMON_TYPES_H */