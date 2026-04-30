/******************************************************************************
 * Middleware/crc_engine.c
 *****************************************************************************/
#include "crc_engine.h"

uint16_t crc16(const uint8_t *d, uint32_t len, uint16_t crc)
{
    for (uint32_t i = 0; i < len; i++) {
        crc ^= (uint16_t)d[i] << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
    }
    return crc;
}

uint32_t crc32(const uint8_t *d, uint32_t len, uint32_t crc)
{
    crc ^= 0xFFFFFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= d[i];
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320) : (crc >> 1);
    }
    return crc ^ 0xFFFFFFFF;
}

