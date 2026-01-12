#include "../include/pico/time.h"
#include <time.h>

static uint64_t get_time_us() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_nsec / 1000 + spec.tv_sec * 1000000;
}

inline int32_t abs(int32_t time) {
    if (time < 0) return -time;
    return time;
}

typedef struct {
    repeating_timer_callback_t callback;
    int32_t delay_ms;
    repeating_timer_t *timer;
    void *user_data;
    uint64_t last_time;
} Timer;

static Timer timers[10]; // TODO ??
static uint32_t num_timers = 0;
static uint64_t boot_time = 0;

void *timer_update(void *_) {
    (void)_;
    boot_time = get_time_us();
    // TODO handle µs case
    // handle user data
    while (true) {
        uint64_t time = get_time_us() / 1000;
        for (int i = 0; i < num_timers; i++) {
            uint64_t time_since = time - timers[i].last_time;
            if (time_since >= abs(timers[i].delay_ms)) {
                if (timers[i].delay_ms < 0) {
                    timers[i].last_time = time;
                }
                bool keep_going = timers[i].callback(timers[i].timer);
                if (timers[i].delay_ms > 0) {
                    timers[i].last_time = get_time_us() / 1000;
                }
            }
        }
    }
    return NULL;
}

alarm_pool_t *alarm_pool_create_with_unused_hardware_alarm(int max_timers) {
    // TODO handle max_timers and lack of unused hardware alarms
    return (void*)0;
}


absolute_time_t get_absolute_time() {
    return get_time_us();
}

uint64_t to_us_since_boot(absolute_time_t t) {
    return t - boot_time;
}
uint64_t to_ms_since_boot(absolute_time_t t) {
    return (t - boot_time) / 1000;
}

void sleep_ms(uint32_t ms) {
    struct timespec ts = (struct timespec){.tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000 };
    nanosleep(&ts, &ts);
}

void sleep_us(uint64_t us) {
    // printf("us: %lu\n", us);
    struct timespec ts = (struct timespec){.tv_sec = us / 1000000, .tv_nsec = (us % 1000000) * 1000 };
    nanosleep(&ts, &ts);
}

bool alarm_pool_add_repeating_timer_ms(alarm_pool_t *pool, int32_t delay_ms, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out) {
    timers[num_timers] = (Timer){
        .callback = callback,
        .delay_ms = delay_ms,
        .timer = out,
        .user_data = user_data,
        .last_time = get_time_us() / 1000,
    };
    num_timers++;
    return true;
}

