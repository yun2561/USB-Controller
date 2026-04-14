/* ---- crc_engine.h ---- */
#ifndef CRC_ENGINE_H
#define CRC_ENGINE_H

#include "common_types.h"

uint16_t crc16_ccitt(const uint8_t *data, uint32_t len,uint16_t init);
uint32_t crc32(const uint8_t *data,uint32_t len,uint32_t init);
#endif /*CRC_ENGINE_H*/