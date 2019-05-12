/*
 * i2c.c
 * Copyright (C) 2019  jasLogic
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "i2c.h"

#include <stdint.h>

#include "peripherals.h"

uint32_t *
i2c_map(void)
{
    if (!peripheral_ismapped((void *)i2c_base_ptr, I2C_SIZE)) {
        i2c_base_ptr = (volatile uint32_t *) peripheral_map(PERIPHERAL_BASE +
            I2C_OFFSET, I2C_SIZE);
    }
	return (uint32_t *)i2c_base_ptr;
}

void
i2c_unmap(void)
{
    peripheral_unmap((void *)i2c_base_ptr, I2C_SIZE);
}

void
i2c_configure(i2c_config_t *config)
{
    I2C->A = config->addr;
    I2C->DIV = config->div;
    I2C->CLKT = config->clkstr;
}

void
i2c_start(void)
{
    I2C->C |= I2C_C_I2CEN; // clear fifo, enable bsc controller
    I2C->S = 0x50; // reset flag register
}
void
i2c_stop(void)
{
    I2C->C &= ~I2C_C_I2CEN; // disable bsc controller
}

inline void
i2c_write_byte(uint8_t byte)
{
    I2C->DLEN = 1; // one byte transfer
    I2C->C |= I2C_C_CLEAR; // clear fifo
    I2C->C &= ~I2C_C_READ; // clear read bit -> write

    I2C->FIFO = byte;
    I2C->C |= I2C_C_ST; // start transfer

    while(!(I2C->S & I2C_S_DONE)); // wait for done
    I2C->S |= I2C_S_DONE; // clear done flag
}

inline uint8_t
i2c_read_byte(void)
{
    I2C->DLEN = 1; // one byte transfer
    I2C->C |= I2C_C_READ | I2C_C_CLEAR; // set read bit, clear FIFO

    I2C->C |= I2C_C_ST; // start transfer
    while(!(I2C->S & I2C_S_DONE)); // wait for done
    I2C->S |= I2C_S_DONE; // clear done flag

    return I2C->FIFO;
}

inline void
i2c_write_data(const uint8_t *data, uint16_t length)
{
    uint16_t i;

    I2C->DLEN = length;
    I2C->C |= I2C_C_CLEAR; // clear fifo
    I2C->C &= ~I2C_C_READ; // clear read bit -> write

    // fill fifo before starting transfer
    for (i = 0; i < length && i < I2C_FIFO_SIZE; ++i) {
        I2C->FIFO = data[i];
    }
    I2C->C |= I2C_C_ST; // start transfer
    while(i < length && I2C->S & I2C_S_TXD) { // write data to fifo while there is data and space
        I2C->FIFO = data[i++];
    }
    while(!(I2C->S & I2C_S_DONE)); // wait for done
    I2C->S |= I2C_S_DONE; // clear done flag
}

inline void
i2c_read_data(uint8_t *data, uint16_t length)
{
    uint16_t i = 0;

    I2C->DLEN = length;
    I2C->C |= I2C_C_READ | I2C_C_CLEAR; // set read bit, clear FIFO

    I2C->C |= I2C_C_ST; // start transfer

    while(!(I2C->S & I2C_S_DONE)) { // while not done
        while(i < length && I2C->S & I2C_S_RXS) { // while data left and fifo not empty
            data[i++] = I2C->FIFO;
        }
    }
    while(i < length && I2C->S & I2C_S_RXS) { // remaining data in fifo
        data[i++] = I2C->FIFO;
    }
    I2C->S |= I2C_S_DONE; // clear done flag
}

/*
 * the difference between this function and 2 i2c_read_byte is that
 * this function is only ONE transmission with two data packets, while
 * 2 times i2c_read_byte makes two individual transmissions which might not
 * work on some ics
 */
inline void
i2c_write_register(uint8_t reg, uint8_t data)
{
    uint8_t tf[2] = {reg, data};
    i2c_write_data(tf, 2);
}

/*
 * different to i2c_write_register, this function can be replaced by
 * one i2c_write_byte and i2c_read_byte
 */
inline uint8_t
i2c_read_register(uint8_t reg)
{
    i2c_write_byte(reg);
    return i2c_read_byte();
}