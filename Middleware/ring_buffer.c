/******************************************************************************
 * Middleware/ring_buffer.c
 *****************************************************************************/
#include "ring_buffer.h"

void rb_init(ring_buf_t *rb)
{
    rb->head = 0;
    rb->tail = 0;

}

uint8_t rb_push(ring_buf_t *rb, uint8_t b)
{
    uint16_t n = (rb->head + 1) & RB_MASK;
    if ( n == rb->tail) return 0;
    rb->buf[rb->head] = b;
    rb->head = n;
    return 1;   
}

uint8_t rb_pop(ring_buf_t *rb, uint8_t *b)
{
    if (rb->head == rb->tail) return 0;
    *b = rb->buf[rb->tail];
    rb->tail = rb->tail + 1 & RB_MASK;
    return 1;
}

uint16_t rb_push_blk(ring_buf_t *rb, const uint8_t*data, uint16_t len)
{
    uint16_t n = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        if(!rb_push(rb,data[i])) break;
        n++;
    }
    return n;
}

uint16_t rb_pop_blk(ring_buf_t *rb, uint8_t *data, uint16_t max)
{
    uint16_t n = 0;
    while ( n < max )
    {
        if (!rb_pop(rb,&data[n])) break;
        n++;
    }
    return n;
    
}

uint16_t rb_avail(const ring_buf_t *rb)
{
    return (rb->head - rb->tail) & RB_MASK;
}
void rb_flush(ring_buf_t *rb)
{
    rb->tail = rb->head; 
}
