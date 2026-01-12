#ifndef __HARDWARE_I2C
#define __HARDWARE_I2C
#include <stdint.h>
#include <stdbool.h>
struct _i2c {
  
};

#define PICO_DEFAULT_I2C_SDA_PIN 1
#define PICO_DEFAULT_I2C_SCL_PIN 1
void i2c_write_blocking(struct _i2c i2c, uint8_t addr, uint8_t *data, uint32_t bytes, bool rescind_bus);
void i2c_read_blocking(struct _i2c i2c, uint8_t addr, uint8_t *data, uint32_t bytes, bool rescind_bus);

void i2c_init(struct _i2c i2c, uint32_t speed);

extern struct _i2c i2c_default;
#endif
