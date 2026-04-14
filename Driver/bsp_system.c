/*============================================================
 * bsp_system.c
 *==========================================================*/
#include "bsp_system.h"


static volatile uint32_t s_tick  = 0;

void bsp_system_init(void)
{
    /* ---- Clock: 48 MHz from IRC48M (USB-compatible) ---- */
    rcu_deinit();
    /* 打开IRC8M */
    rcu_osci_on(RCU_IRC8M);
    while (SUCCESS != rcu_osci_stab_wait(RCU_IRC8M));

    /* 配PLL：8M /2 *12 = 48MHz */
    rcu_pll_config(RCU_PLLSRC_IRC8M_DIV2, RCU_PLL_MUL12);

    rcu_osci_on(RCU_PLL_CK);
    while (SUCCESS != rcu_osci_stab_wait(RCU_PLL_CK));

    /* 切换系统时钟到PLL */
    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);

    /* AHB = 48 MHz, APB1 = 48 MHz, APB2 = 48 MHz */
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV1);
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
    
    SysTick_Config(SystemCoreClock / SYSTICK_FREQ_HZ);

    /* Enable peripheral clocks */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_DMA);
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_I2C0);
    rcu_periph_clock_enable(RCU_ADC);
    rcu_periph_clock_enable(RCU_TIMER0);
    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_periph_clock_enable(RCU_TIMER2);

    bsp_nvic_config();
    //dbg_deinit();
}

void bsp_nvic_config()
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    nvic_irq_enable(USBFS_IRQn,0,0);

    nvic_irq_enable(DMA_Channel1_2_IRQn,0,0);

    nvic_irq_enable(USART0_IRQn,0,0);

    nvic_irq_enable(EXTI4_15_IRQn, 2, 0);

    nvic_irq_enable(SysTick_IRQn, 2, 0);

    nvic_irq_enable(ADC_CMP_IRQn, 3, 0);

    nvic_irq_enable(DMA_Channel3_4_IRQn, 4, 0);
}


void bsp_system_reset(void) { NVIC_SystemReset();}

uint32_t bsp_get_tick(void) { return s_tick;}

void bsp_delay_ms(uint32_t ms) { uint32_t s = s_tick; while ((s_tick-s) < ms) ;}

void bsp_systick_increment(void) { s_tick++; }
