/*============================================================
 * bsp_uart_midi.h
 *==========================================================*/

#ifndef BSP_UART_MIDI_H
#define BSP_UART_MIDI_H

#include "common_types.h"

#define UART_RX_BUF_SIZE  256U
#define UART_TX_BUF_SIZE  256U

void        bsp_uart_midi_init(void);
void        bsp_uart_midi_send_dma(const uint8_t *data, uint16_t len);
uint8_t     bsp_uart_midi_tx_busy(void);
uint16_t    bsp_uart_midi_rx_available(void);
uint8_t     bsp_uart_midi_rx_read(void);


#endif /* BSP_UART_MIDI_H */
