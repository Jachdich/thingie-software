#ifndef TESTING
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
// #include "i2s.pio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../include/i2s.h"
#include "../include/sgtl5000.h"
#include "../include/wheel.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define I2C_ADDR 0x0A

uint32_t sgtl5000_read(uint16_t reg) {
    uint8_t packets[] = {reg >> 8, reg & 0xFF};
    uint8_t bytes_written = i2c_write_blocking(i2c0, I2C_ADDR, packets, 2, true);
    if (bytes_written != 2) {
        return 0;
    }

    uint8_t buf[2];
    uint32_t bytes_read = i2c_read_blocking(i2c0, I2C_ADDR, buf, 2, false) << 8;
    if (bytes_read != 2) {
        return 0;
    }
    return buf[0] << 8 | buf[1];
}

size_t sgtl5000_write(uint16_t reg, uint16_t val) {
    uint8_t packets[] = {reg >> 8, reg & 0xFF, val >> 8, val & 0xFF};
    uint8_t bytes = i2c_write_blocking(i2c0, I2C_ADDR, packets, 4, false);
    if ((int8_t)bytes == PICO_ERROR_GENERIC) {
        return 0;
    }
    return bytes;
}

bool sgtl5000_setup(void) {
    i2c_init(i2c0, 10000); // TODO speed?
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_pulls(PICO_DEFAULT_I2C_SDA_PIN, true, false);
    gpio_set_pulls(PICO_DEFAULT_I2C_SCL_PIN, true, false);
    gpio_set_drive_strength(PICO_DEFAULT_I2C_SDA_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(PICO_DEFAULT_I2C_SCL_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_slew_rate(PICO_DEFAULT_I2C_SDA_PIN, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(PICO_DEFAULT_I2C_SCL_PIN, GPIO_SLEW_RATE_FAST);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    printf("i2c init\n");
    sleep_ms(5);
    size_t r = sgtl5000_write(CHIP_ANA_POWER, ANA_POWER_REFTOP_POWERUP |
                                              ANA_POWER_ADC_MONO |
                                              ANA_POWER_DAC_MONO);
    printf("wrote first value, ret: %d\n", r);
    if (!r) {
        return false;
    }
    sgtl5000_write(CHIP_LINREG_CTRL, LINREG_VDDC_VOLTAGE(0x0C /*1.0V*/) |
                                       LINREG_VDDC_FROM_VDDIO |
                                       LINREG_VDDC_SOURCE_OVERRIDE);
    sgtl5000_write(CHIP_REF_CTRL, REF_VAG_VOLTAGE(0x1F /*1.575V*/) |
                                    REF_BIAS_CTRL(0x1 /*+12.5%*/));
    sgtl5000_write(CHIP_LINE_OUT_CTRL,
                 LINE_OUT_CTRL_VAG_VOLTAGE(0x22 /*1.65V*/) |
                     LINE_OUT_CTRL_OUT_CURRENT(0x3C /*0.54mA*/));
    sgtl5000_write(CHIP_SHORT_CTRL, 0x4446); // fuck it, I cba to work this out
    sgtl5000_write(CHIP_ANA_CTRL, 0x0137);   // enable zero cross detectors

    // TODO: Handle case if SGTL5000 is the I2S master. For now, assume slave
    sgtl5000_write(CHIP_ANA_POWER,
                 ANA_POWER_CAPLESS_HEADPHONE_POWERUP | ANA_POWER_DAC_POWERUP |
                     ANA_POWER_HEADPHONE_POWERUP | ANA_POWER_DAC_MONO |
                     ANA_POWER_VAG_POWERUP | ANA_POWER_REFTOP_POWERUP);
    sgtl5000_write(CHIP_DIG_POWER, DIG_POWER_DAC | DIG_POWER_I2S_IN);
    sleep_ms(400);
    sgtl5000_write(CHIP_CLK_CTRL, 0x0008); // 48 kHz, 256*Fs
    sgtl5000_write(CHIP_I2S_CTRL, 0x0000); // SCLK=64*Fs, 32bit, I2S format
    // default signal routing is ok?
    sgtl5000_write(CHIP_SSS_CTRL, 0x0010);    // ADC->I2S, I2S->DAC
    sgtl5000_write(CHIP_ADCDAC_CTRL, 0x0000); // disable dac mute
    sgtl5000_write(CHIP_DAC_VOL, 0x3C3C);     // digital gain, 0dB
    sgtl5000_write(CHIP_ANA_HP_CTRL, 0x7F7F); // set volume (lowest level)
    sgtl5000_write(CHIP_ANA_CTRL, 0x0026);    // enable zero cross detectors
    return true;
}

// void sgtl_mute_dac(bool muted) {
//     // 32
// }

uint16_t calc_volume(float volume, uint16_t range) {
    uint16_t cvol = (uint16_t)((volume * (float)range) + .499);
    return (cvol > range) ? range : cvol;
}

void sgtl_volume(float left, float right) {
    if (left < 0.0 || right < 0.0 || left > 1.0 || right > 1.0) return;

    uint16_t left_bits = 0x007F - calc_volume(left, 0x7F);
    uint16_t right_bits = 0x007F - calc_volume(right, 0x7F);
    uint16_t volume = (right_bits << 8) | left_bits;
    sgtl5000_write(CHIP_ANA_HP_CTRL, volume);
}

void sgtl_dac_volume(float left, float right) {
    // Make sure the volume is within thew proper range.
    // Range is from 0x3c (0 dB) to 0xFC (muted)
    if (left < 0.0 || right < 0.0 || left > 1.0 || right > 1.0) return;

    uint16_t right_channel = 0x00FC - calc_volume(right, 0x00C0);
    uint16_t left_channel  = 0x00FC - calc_volume(left,  0x00C0);
    uint16_t volume = (right_channel << 8) | (left_channel << 0);
    sgtl5000_write(CHIP_DAC_VOL, volume);
}
// auto sgtl5000::headphone_select(uint16_t input) -> void {
//     // Make sure the input is within the proper range.
//     if ((input != AUDIO_HEADPHONE_DAC) && (input != AUDIO_HEADPHONE_LINEIN))
//         return;

//     if (input == AUDIO_HEADPHONE_DAC)
//         analog_ctrl_ &= ~(1 << 6);
//     else if (input == AUDIO_HEADPHONE_LINEIN)
//         analog_ctrl_ |= (1 << 6);
//     write_reg(CHIP_ANA_CTRL, analog_ctrl_);

// const static int AUDIO_SAMPLE_RATE_EXACT = 44100.0f;
static __attribute__((aligned(8))) pio_i2s i2s;
const float f_s = 48000;

#define BUFFER_SIZE (1152 * 7)
volatile int16_t *samples;
volatile uint32_t read_ptr;
volatile uint32_t write_ptr;

static void process_audio(
    const int32_t* input, 
    int32_t* output, 
    size_t num_frames) 
{
    (void)input;
    // Values being sent to the codec are 32 bit signed 
    // values but the samples are only 16 bits so they
    // need to be pushed to the left edge.
    //
    for (size_t i = 0; i < num_frames; i++)
    {
        output[2 * i + 0] = (int32_t)samples[read_ptr++] << 16;      // right channel
        output[2 * i + 1] = (int32_t)samples[read_ptr++] << 16;      // left channel
        read_ptr %= BUFFER_SIZE;
    }
}


/**
 * @brief Handler for when the DMA is ready
 */
static void dma_i2s_in_handler() 
{
    // We're double buffering using chained TCBs. By checking which buffer the
    // DMA is currently reading from, we can identify which buffer it has just
    // finished reading (the completion of which has triggered this interrupt).
    //
    if (*(int32_t**)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer) 
    {
        // It is inputting to the second buffer so we can overwrite the first
        //
        process_audio(
            i2s.input_buffer, 
            i2s.output_buffer, 
            AUDIO_BUFFER_FRAMES);
    } 
    else 
    {
        // It is currently inputting the first buffer, so we write to the second
        //
        process_audio(
            &i2s.input_buffer[STEREO_BUFFER_SIZE], 
            &i2s.output_buffer[STEREO_BUFFER_SIZE], 
            AUDIO_BUFFER_FRAMES);
    }
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data;  // clear the IRQ
}

#define FPM_ARM
#include "../lib/libmad/mad.h"
// #include "../include/mp32.h"
#include "../include/drawing.h"
uint8_t mp3[100];
uint32_t mp3_len = 100;


struct MusicState {
    struct mad_stream stream;
    struct mad_frame frame;
    struct mad_synth synth;
    float volume;
    int16_t samples[BUFFER_SIZE];
};

void music_init(struct MusicState *state) {
    state->volume = 0.1;

    mad_stream_init(&state->stream);
    mad_frame_init(&state->frame);
    mad_synth_init(&state->synth);
    mad_stream_options(&state->stream, 0);

    mad_stream_buffer(&state->stream, mp3, mp3_len);

    i2s_config i2s_cfg = (i2s_config){ .fs = 48000, .sck_mult = 256, .bit_depth=32, .sck_pin = 8, .dout_pin = 9, .din_pin = 5, .clock_pin_base = 6, .sck_enable = true};
    i2s_program_start_synched(pio0, &i2s_cfg, dma_i2s_in_handler, &i2s);
    sleep_ms(10);
    bool success = sgtl5000_setup();
    printf("Powered up success? %d\n", success);
    sgtl_dac_volume(1.0, 1.0);
    sgtl_volume(state->volume, state->volume);
    samples = state->samples;

}

int music_step(struct MusicState *state, Screen s) {
    float len_f = (write_ptr - read_ptr) % BUFFER_SIZE;
    int len = len_f / BUFFER_SIZE * 200;
    draw_rect(s, vec2(0, 110), vec2(len, 20), 0xffff);

    uint32_t num = (write_ptr - read_ptr) % BUFFER_SIZE;
    int n = 0;
    while (num < BUFFER_SIZE - 1152 * 4) {
        long time = to_us_since_boot(get_absolute_time());
        mad_frame_decode(&state->frame, &state->stream);
        mad_synth_frame(&state->synth, &state->frame);
        for (int i = 0; i < 1152; i++) {
            state->samples[write_ptr++] = state->synth.pcm.samplesX[i][0];
            state->samples[write_ptr++] = state->synth.pcm.samplesX[i][1];
            write_ptr %= BUFFER_SIZE;
        }
        num = (write_ptr - read_ptr) % BUFFER_SIZE;
        long time2 = to_us_since_boot(get_absolute_time());
        long dur = time2 - time;
        printf("%lu\n", dur);
        n += 1;
    }
    printf("n%d\n", n);
    len_f = (write_ptr - read_ptr) % BUFFER_SIZE;
    len = len_f / BUFFER_SIZE * 200;
    draw_rect(s, vec2(0, 140), vec2(len, 20), 0xffff);
    draw_yline(s, vec2(200, 0), 240, 0xffff);

    float delta = get_wheel_delta() / 4096.0;
    state->volume += delta;
    if (state->volume < 0.0) state->volume = 0.0;
    if (state->volume > 1.0) state->volume = 1.0;

    sgtl_volume(state->volume, state->volume);

    return 1;
}

#endif
// 6ms
