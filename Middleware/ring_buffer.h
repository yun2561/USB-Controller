/******************************************************************************
 * Middleware/ring_buffer.h
 *****************************************************************************/

#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include "common_types.h"

#define RB_SIZE RING_SIZE       /* From common_types.h = 256 */
#define RB_MASK (RB_SIZE - 1)
typedef struct{
    volatile uint16_t head;
    volatile uint16_t tail;
    uint8_t buf[RB_SIZE];
} ring_buf_t;

void        rb_init(ring_buf_t *rb);
uint8_t        rb_push(ring_buf_t *rb, uint8_t b);
uint8_t        rb_pop(ring_buf_t *rb, uint8_t *b);
uint16_t    rb_push_blk(ring_buf_t *rb, const uint8_t*data, uint16_t len);
uint16_t    rb_pop_blk(ring_buf_t *rb, uint8_t *data, uint16_t max);
uint16_t    rb_avail(const ring_buf_t *rb);
void        rb_flush(ring_buf_t *rb);



#endif /* RING_BUFFER_H */

