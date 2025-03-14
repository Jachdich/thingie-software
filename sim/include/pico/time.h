#ifndef _PICO_TIME_H
#define _PICO_TIME_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {} repeating_timer_t;
typedef struct {} alarm_pool_t;
typedef uint64_t absolute_time_t;

typedef bool(* repeating_timer_callback_t) (repeating_timer_t *rt);

alarm_pool_t *alarm_pool_create_with_unused_hardware_alarm(int max_timers);
bool alarm_pool_add_repeating_timer_ms (alarm_pool_t *pool, int32_t delay_ms, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out);
absolute_time_t get_absolute_time(void);
uint64_t to_us_since_boot(absolute_time_t t);
uint64_t to_ms_since_boot(absolute_time_t t);
void sleep_ms(uint32_t ms);
void sleep_us(uint64_t us);
#endif