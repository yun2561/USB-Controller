/******************************************************************************
 * Driver/bsp_fmc.c
 *****************************************************************************/

#include "bsp_fmc.h"


fmc_st_t bsp_fmc_erase(uint32_t a)
{
    if (a < APP_ADDR || a > CONFIG_ADDR -1) return FMC_ERR_WRP;
    fmc_unlock(); 
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    fmc_state_enum s = fmc_page_erase(a);
    fmc_lock();
    return s == FMC_READY ? FMC_OK : FMC_ERR_PG;
}

fmc_st_t bsp_fmc_prog(uint32_t a, const uint32_t *d, uint32_t w)
{
    if (a < APP_ADDR || (a + w *4) > CONFIG_ADDR) return FMC_ERR_WRP;
    fmc_unlock();
    for (uint32_t i = 0; i < w; i++)
    {
        if (fmc_word_program(a + i * 4, d[i]) != FMC_READY)
        {
            fmc_lock();
            return FMC_ERR_PG;
        }
        fmc_lock();
        
    }
    return FMC_OK;
}

uint8_t bsp_fmc_verify(uint32_t a, const uint32_t *d, uint32_t w)
{
    for (uint32_t i =0; i < w; i++)
        if (*(volatile uint32_t *)(a + i * 4) != d[i]) return 0;
    return 1;
}



