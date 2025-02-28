cmake_minimum_required(VERSION 3.13...3.27)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_definitions("-pedantic -W -Wall -Wunreachable-code  -Wwrite-strings -Wpointer-arith -Wbad-function-cast -Wcast-align -Wcast-qual")

add_executable(main
    src/main.c
    src/st7789_pio.c
    src/keypad.c
    # src/st7789.c
    src/snake.c
    src/minesweeper.c
    src/drawing.c

    lib/mcufont/mf_bwfont.c
    lib/mcufont/mf_font.c
    lib/mcufont/mf_kerning.c
    lib/mcufont/mf_scaledfont.c
    lib/mcufont/mf_encoding.c
    lib/mcufont/mf_justify.c
    lib/mcufont/mf_rlefont.c
    lib/mcufont/mf_wordwrap.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib hardware_i2c hardware_pio hardware_clocks hardware_dma hardware_spi hardware_pwm pico_multicore)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/src/st7789_lcd.pio)