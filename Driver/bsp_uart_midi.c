/*============================================================
 * bsp_uart_midi.c
 *==========================================================*/
#include "bsp_uart_midi.h"
#include "gd32f3x0.h"
#include <stdlib.h>
#include <string.h>

//#include "debug_rtt.h"
//#include "error_handler.h"

//#define UART_RX_BUF_SIZE  RING_SIZE
//#define UART_TX_BUF_SIZE  RING_SIZE
#define RX_MASK           (UART_RX_BUF_SIZE - 1)

static uint8_t s_rx_buf[UART_RX_BUF_SIZE];
static volatile uint16_t s_rx_head = 0;
static uint16_t s_rx_tail = 0;
static uint8_t s_tx_buf[UART_TX_BUF_SIZE];
static volatile uint8_t s_tx_busy = 0;

void bsp_uart_midi_init(void)
{
    gpio_af_set             (MIDI_TX_PORT, MIDI_TX_AF, MIDI_TX_PIN);
    gpio_af_set             (MIDI_RX_PORT, MIDI_RX_AF, MIDI_RX_PIN);
    gpio_mode_set           (MIDI_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, MIDI_TX_PIN | MIDI_RX_PIN);
    gpio_output_options_set (MIDI_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, MIDI_TX_PIN);

    usart_deinit            (MIDI_UART);
    usart_baudrate_set      (MIDI_UART, MIDI_UART_BAUD);
    usart_word_length_set   (MIDI_UART, USART_WL_8BIT);
    usart_stop_bit_set      (MIDI_UART, USART_STB_1BIT);
    usart_parity_config     (MIDI_UART, USART_PM_NONE);
    usart_transmit_config   (MIDI_UART, USART_TRANSMIT_ENABLE);
    usart_receive_config    (MIDI_UART, USART_RECEIVE_ENABLE);
    usart_interrupt_enable  (MIDI_UART, USART_INT_IDLE);

    /* DMA RX: MIDI_RX_DMA_CH (CH2), circular */
    dma_deinit              (MIDI_RX_DMA_CH);
    dma_parameter_struct rx_dma;
    dma_struct_para_init(&rx_dma);
    rx_dma.periph_addr  = (uint32_t)&USART_RDATA(MIDI_UART);
    rx_dma.memory_addr  = (uint32_t)s_rx_buf;
    rx_dma.direction    = DMA_PERIPHERAL_TO_MEMORY;
    rx_dma.number       = UART_RX_BUF_SIZE;
    rx_dma.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    rx_dma.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    rx_dma.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    rx_dma.memory_width = DMA_MEMORY_WIDTH_8BIT;
    rx_dma.priority     = DMA_PRIORITY_HIGH;
    dma_init                    (MIDI_RX_DMA_CH, &rx_dma);
    dma_circulation_enable      (MIDI_RX_DMA_CH);
    dma_channel_enable          (MIDI_RX_DMA_CH);
    usart_dma_receive_config    (MIDI_UART, USART_RECEIVE_DMA_ENABLE);

    /* DMA TX: MIDI_TX_DMA_CH (CH1), normal mode — full config */
    dma_deinit(MIDI_TX_DMA_CH);
    dma_parameter_struct tx;
    dma_struct_para_init(&tx);
    tx.periph_addr  = (uint32_t)&USART_TDATA(MIDI_UART);
    tx.memory_addr  = (uint32_t)s_tx_buf;
    tx.direction    = DMA_MEMORY_TO_PERIPHERAL;
    tx.number       = 0;  /* Will be set per-transfer */
    tx.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    tx.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    tx.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    tx.memory_width = DMA_MEMORY_WIDTH_8BIT;
    tx.priority     = DMA_PRIORITY_HIGH;
    dma_init(MIDI_TX_DMA_CH, &tx);
    dma_interrupt_enable(MIDI_TX_DMA_CH, DMA_INT_FTF);
    usart_dma_transmit_config(MIDI_UART, USART_TRANSMIT_DMA_ENABLE);

    usart_enable(MIDI_UART);

}

void bsp_uart_midi_send_dma(const uint8_t *data, uint16_t len)
{
    if (!len || len > UART_TX_BUF_SIZE) return;

    while (s_tx_busy);
    memcpy (s_tx_buf, data, len);
    s_tx_busy = 1;

    dma_channel_disable         (MIDI_TX_DMA_CH);
    dma_memory_address_config   (MIDI_TX_DMA_CH, (uint32_t)s_tx_buf);
    dma_transfer_number_config  (MIDI_TX_DMA_CH, len);
    dma_channel_enable          (MIDI_TX_DMA_CH);
}


uint8_t bsp_uart_midi_tx_busy(void) { return s_tx_busy; }

uint16_t bsp_uart_midi_rx_available(void)
{
    s_rx_head = UART_RX_BUF_SIZE - dma_transfer_number_get(MIDI_RX_DMA_CH);
    return (s_rx_head - s_rx_tail) & RX_MASK;
}

uint8_t bsp_uart_midi_rx_read(void)
{
    uint8_t b = s_rx_buf[s_rx_tail];
    s_rx_tail = (s_rx_tail + 1) & RX_MASK;
    return b;
} 
/* IRQ: USART IDLE */
void USART0_IRQHander(void)
{
    if (usart_interrupt_flag_get(MIDI_UART,USART_INT_FLAG_IDLE))
    {
        (void)USART_STAT(MIDI_UART);
        (void)USART_RDATA(MIDI_UART);
        s_rx_head = UART_RX_BUF_SIZE - dma_transfer_number_get(MIDI_RX_DMA_CH);
    }
}

/* IRQ: DMA CH1/CH2 (TX complete) */
void DMA_Channel1_2_IRQHandler(void)
{
    if (dma_interrupt_flag_get(MIDI_TX_DMA_CH,DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(MIDI_TX_DMA_CH,DMA_INT_FLAG_FTF);
        dma_channel_disable(MIDI_TX_DMA_CH);
        s_tx_busy = 0;
    }
}






