/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#include "st7789_pio.h"

// Tested with the parts that have the height of 240 and 320

#define PIN_DIN 19
#define PIN_CLK 18
#define PIN_CS 28
#define PIN_DC 20
#define PIN_RESET 21
#define PIN_BL 26

#define SERIAL_CLK_DIV 1.f

#define asb 80
#define asa 320

#define bsb 0
#define bsa 240

// static spi_t spi = {
//     .hw_inst = spi0,  // SPI component
//     .sck_gpio = 18,    // GPIO number (not Pico pin number)
//     .mosi_gpio = 19,
//     .miso_gpio = 16,
//     .baud_rate = 125 * 1000 * 1000 / 16  // 15625000 Hz
//     //.baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
//     // .baud_rate = 125 * 1000 * 1000 / 4  // 31250000 Hz
//     //.baud_rate = 125 * 1000 * 1000 / 2  // 62500000 Hz
// };

// Format: cmd length (including cmd byte), post delay in units of 5 ms, then cmd payload
// Note the delays have been shortened a little
static const uint8_t st7789_init_seq[] = {
        1, 20, 0x01,                        // Software reset
        1, 10, 0x11,                        // Exit sleep mode
        2, 2, 0x3a, 0b01010101,             // Set colour mode to 16 bit
        2, 2, 0xc6, 0x18, // frame rate 46fps
        2, 0, 0x36, 0b10100000,             // Set MADCTL: column then row, reflect y, refresh is bottom to top ????
        5, 0, 0x2a, asb >> 8, asb & 0xff, asa >> 8, asa & 0xff,   // CASET: column addresses
        5, 0, 0x2b, bsb >> 8, bsb & 0xff, bsa >> 8, bsa & 0xff, // RASET: row addresses
        1, 2, 0x21,                         // Inversion on, then 10 ms delay (supposedly a hack?)
        1, 2, 0x13,                         // Normal display on, then 10 ms delay
        1, 10, 0x29,                        // Main screen turn on, then wait 100 ms
        2, 10, 0xb7, 0x70,                  // some hackery to get somewhat better blacks out of the display...
        
        // 1, 1, 0x28,
        0                                   // Terminate list
};

void lcd_set_dc_cs(bool dc, bool cs) {
    sleep_us(1);
    gpio_put_masked((1u << PIN_DC) | (1u << PIN_CS), !!dc << PIN_DC | !!cs << PIN_CS);
    sleep_us(1);
}

static inline void lcd_write_cmd(const uint8_t *cmd, size_t count) {
    // st7789_lcd_wait_idle();
    lcd_set_dc_cs(0, 0);
    st7789_lcd_put(*cmd++);
    if (count >= 2) {
        // st7789_lcd_wait_idle(pio, sm);
        lcd_set_dc_cs(1, 0);

        spi_write_blocking(spi0, cmd, count - 1);
        // for (size_t i = 0; i < count - 1; ++i)
        //     st7789_lcd_put(*cmd++);
    }
    // st7789_lcd_wait_idle(pio, sm);
    lcd_set_dc_cs(1, 1);
}

static inline void lcd_init(const uint8_t *init_seq) {
    const uint8_t *cmd = init_seq;
    while (*cmd) {
        lcd_write_cmd(cmd + 2, *cmd);
        sleep_ms(*(cmd + 1) * 5);
        cmd += *cmd + 2;
    }
}

void st7789_lcd_put(uint8_t v) {
    spi_write_blocking(spi0, &v, 1);
}

#ifndef TESTING
void st7789_start_pixels() {
    uint8_t cmd = 0x2c; // RAMWR
    lcd_write_cmd(&cmd, 1);
    lcd_set_dc_cs(1, 0);
}
#endif

void st7789_init() {

    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RESET);
    gpio_init(PIN_BL);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_RESET, GPIO_OUT);


    const uint sck_gpio = 18;    // GPIO number (not Pico pin number)
    const uint mosi_gpio = 19;
    const uint miso_gpio = 16;
    spi_init(spi0, 700 * 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(miso_gpio, GPIO_FUNC_SPI);
    gpio_set_function(mosi_gpio, GPIO_FUNC_SPI);
    gpio_set_function(sck_gpio, GPIO_FUNC_SPI);

    gpio_put(PIN_CS, 1);
    gpio_put(PIN_RESET, 1);

    lcd_init(st7789_init_seq);
    spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}
