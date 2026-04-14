/*============================================================
 * bsp_system.h
 *==========================================================*/
#ifndef BSP_SYSTEM_H
#define BSP_SYSTEM_H

#include "gd32f3x0.h"
#include "common_types.h"


void bsp_system_init(void);
void bsp_nvic_config(void);
void bsp_boot_check(void);
void bsp_system_reset(void);
uint32_t bsp_get_tick(void);
void bsp_delay_ms(uint32_t ms);   



#endif /* BSP_SYSTEM_H */