#include "crc_engine.h"

uint16_t crc16_ccitt(const uint8_t *data, uint32_t len,uint16_t init)
{
    for(uint32_t i=0;i<len;i++)
    {   init^=(uint16_t)data[i]<<8;
        for(int j=0;j<8;j++) init=(init&0x8000)?((init<<1)^0x1021):(init<<1);
    }
    return init;
}
uint32_t crc32(const uint8_t *data,uint32_t len,uint32_t init)
{
    init^=0xFFFFFFFF;
    for(uint32_t i=0;i<len;i++)
    {
        init^=data[i];
        for(int j=0;j<8;j++) init=(init&1)?((init>>1)^0xEDB88320):(init>>1);
    }
    return init^0xFFFFFFFF;
}