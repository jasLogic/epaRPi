#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define I2C_OFFSET  0x804000 // BSC1
#define I2C_SIZE    0x20 // ??

volatile uint32_t *i2c_base_ptr;

struct i2c_register_map {
    uint32_t C;
    uint32_t S;
    uint32_t DLEN;
    uint32_t A;
    uint32_t FIFO;
    uint32_t DIV;
    uint32_t DEL;
    uint32_t CLKT;
};
#define I2C     ((struct i2c_register_map *)i2c_base_ptr)

typedef struct {
    uint8_t addr: 7;
    uint16_t div;
} i2c_config_t;

uint32_t *  i2c_map(void);
void        i2c_unmap(void);

void i2c_configure(i2c_config_t *config);

void    i2c_start(void);
void    i2c_stop(void);

extern void     i2c_write_byte(uint8_t byte);
extern uint8_t  i2c_read_byte(void);

extern void     i2c_write_data(const uint8_t *data, uint16_t length);
extern void     i2c_read_data(uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _I2C_H_ */
