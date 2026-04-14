/*============================================================
 * bsp_uart_midi.c
 *==========================================================*/
#include "bsp_uart_midi.h"
#include "gd32f3x0.h"
//#include "debug_rtt.h"
//#include "error_handler.h"

static uint8_t s_rx_buf[UART_RX_BUF_SIZE];
static volatile uint16_t s_rx_head = 0;
static uint16_t s_rx_tail = 0;
static uint8_t s_tx_buf[UART_TX_BUF_SIZE];
static volatile uint8_t s_tx_busy = 0;

void bsp_uart_midi_init(void)
{
    gpio_af_set             (GPIOA, GPIO_AF_1, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_mode_set           (GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set (GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    usart_deinit            (USART0);
    usart_baudrate_set      (USART0, MIDI_UART_BAUDRATE);
    usart_word_length_set   (USART0, USART_WL_8BIT);
    usart_stop_bit_set      (USART0, USART_STB_1BIT);
    usart_parity_config     (USART0, USART_PM_NONE);
    usart_transmit_config   (USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config    (USART0, USART_RECEIVE_ENABLE);
    usart_interrupt_enable  (USART0, USART_INT_IDLE);

    /* DMA CH3: RX circular */
    dma_deinit(DMA_CH3);
    dma_parameter_struct rx_dma;
    dma_struct_para_init(&rx_dma);
    rx_dma.periph_addr  = (uint32_t)&USART_RDATA(USART0);
    rx_dma.memory_addr  = (uint32_t)s_rx_buf;
    rx_dma.direction    = DMA_PERIPHERAL_TO_MEMORY;
    rx_dma.number       = UART_RX_BUF_SIZE;
    rx_dma.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    rx_dma.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    rx_dma.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    rx_dma.memory_width = DMA_MEMORY_WIDTH_8BIT;
    rx_dma.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA_CH3, &rx_dma);
    dma_circulation_enable(DMA_CH3);
    dma_channel_enable(DMA_CH3);
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);

    /* DMA CH2: TX normal */
    dma_deinit(DMA_CH2);
    dma_interrupt_enable(DMA_CH2, DMA_INT_FTF);
    usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);

    usart_enable(USART0);

}

void bsp_uart_midi_send_dma(const uint8_t *data, uint16_t len)
{
    if (!len || len > UART_TX_BUF_SIZE) return;
}



