#ifndef TESTING
#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

#define AS5600_ADDR 0x36

uint16_t as5600_read_raw_angle() {
    uint8_t ret[2];
    uint8_t reg = 0x0E;
    i2c_write_blocking(i2c_default, AS5600_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, AS5600_ADDR, ret, 2, false);
    return (ret[0] << 8 | ret[1]);
}

uint16_t as5600_read_adc() {
    uint8_t ret[2];
    uint8_t reg = 0x1A;
    i2c_write_blocking(i2c_default, AS5600_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, AS5600_ADDR, ret, 2, false);
    return ret[0] << 8 | ret[1];
}
#endif
