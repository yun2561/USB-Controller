/* ---- crc_engine.h ---- */
#ifndef CRC_ENGINE_H
#define CRC_ENGINE_H

#include "common_types.h"

uint16_t crc16(const uint8_t *d, uint32_t len, uint16_t crc);
uint32_t crc32(const uint8_t *d, uint32_t len, uint32_t crc);
#endif /*CRC_ENGINE_H*/