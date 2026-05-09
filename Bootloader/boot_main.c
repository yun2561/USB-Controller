/******************************************************************************
 * Bootloader/boot_main.c
 *
 * Standalone 8KB bootloader. Separate Keil project.
 * Linker: ROM = 0x08000000 (8KB), RAM = 0x20000000 (16KB)
 *
 * Boot flow:
 *   1. Check DFU flag at DFU_STAGING_ADDR
 *      → UPGRADE: clear flag, enter DFU wait
 *   2. Check App validity (SP sanity)
 *      → Valid:   set VTOR, jump to App
 *      → Invalid: enter DFU wait
 *****************************************************************************/

//#define BUILD_BOOTLOADER

#ifdef BUILD_BOOTLOADER

#include "gd32f3x0.h"
#include "common_types.h"

static void jump_to_app(void)
{
    uint32_t app_sp = *(volatile uint32_t *)APP_ADDR;
    uint32_t app_pc = *(volatile uint32_t *)(APP_ADDR + 4U);

    /* SP must point into SRAM (0x20000000 range) */
    if ((app_sp & 0x2FFE0000U) != 0x20000000U) {
        // APP 无效，留在 Bootloader / 进入 DFU
        return;
    }
    // 关闭所有中断、外设
    //__disable_irq();
    // 可选：DeInit USB、UART、Timer 等

    /* Relocate vector table */
    SCB->VTOR = APP_ADDR;

    __set_MSP(app_sp);
    ((void (*)(void))app_pc)();
}

static uint8_t app_is_valid(void)
{
    uint32_t sp = *(volatile uint32_t *)APP_ADDR;
    return ((sp & 0x2FFE0000U) == 0x20000000U) ? 1U : 0U;
}

static uint8_t dfu_flag_set(void)
{
    return (*(volatile uint32_t*)DFU_STAGING_ADDR == DFU_MAGIC_UPGRADE) ? 1U : 0U;
}

static void clear_dfu_flag(void)
{
    fmc_unlock();
    fmc_page_erase(DFU_STAGING_ADDR);
    fmc_lock();
}

static void dfu_wait_loop(void)
{
    /* Minimal indicator: toggle an LED or just wait.
     * In a real product, could implement USB-DFU Class here.
     * Or simply wait for power cycle after App-mode SysEx DFU. */
    while (1)
    {
        __NOP();
    }
    
}

int main(void)
{
    SystemInit();

    if (dfu_flag_set()) {
        clear_dfu_flag();
        dfu_wait_loop();
    }

    if (app_is_valid()) {
        jump_to_app();
    }

    /* No valid app — enter DFU wait */
    dfu_wait_loop();

    return 0; /* Never reached */

}



#endif /* BUILD_BOOTLOADER */

