/******************************************************************************
 * Driver/bsp_fmc.h
 *****************************************************************************/

#ifndef BSP_FMC_H
#define BSP_FMC_H

#include "common_types.h"



typedef enum { FMC_OK , FMC_ST_BUSY, FMC_ERR_PG, FMC_ERR_WRP } fmc_st_t;

fmc_st_t bsp_fmc_erase(uint32_t addr);
fmc_st_t bsp_fmc_prog(uint32_t addr, const uint32_t *d, uint32_t words);
uint8_t  bsp_fmc_verify(uint32_t addr, const uint32_t *d, uint32_t words);



#endif /* BSP_FMC_H */

