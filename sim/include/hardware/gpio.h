#ifndef _HARDWARE_GPIO_H
#define _HARDWARE_GPIO_H
enum gpio_function_rp2350 { GPIO_FUNC_HSTX = 0, GPIO_FUNC_SPI = 1, GPIO_FUNC_UART = 2, GPIO_FUNC_I2C = 3, GPIO_FUNC_PWM = 4, GPIO_FUNC_SIO = 5, GPIO_FUNC_PIO0 = 6, GPIO_FUNC_PIO1 = 7, GPIO_FUNC_PIO2 = 8, GPIO_FUNC_GPCK = 9, GPIO_FUNC_XIP_CS1 = 9, GPIO_FUNC_CORESIGHT_TRACE = 9, GPIO_FUNC_USB = 10, GPIO_FUNC_UART_AUX = 11, GPIO_FUNC_NULL = 0x1f };
#define GPIO_OUT 0
#define GPIO_IN 1

void stdio_init_all();
void gpio_set_dir();
void gpio_set_function();
int pwm_gpio_to_slice_num();
void pwm_set_wrap();
void pwm_set_enabled();
void gpio_init();
void gpio_pull_up();
void gpio_put(int pin, int state);
int gpio_get(int pin);
void gpio_put_masked();
void st7789_lcd_wait_idle();
unsigned int pio_add_program();
void st7789_lcd_program_init();
#endif
