#include <stdint.h>
#include "../include/as5600.h"

uint16_t last_wheel;
int16_t delta;
void wheel_next_frame() {
    uint16_t wheel = as5600_read_raw_angle();
    // ((n % M) + M) % M : https://stackoverflow.com/a/1907585
    delta = ((wheel - last_wheel) % 4096 + 4096) % 4096;
    // we assume the delta is the small way around the circle.
    // if the value wraps, then delta might be very large and it's more appropriate
    // to put it in the range (-2048, 2048)
    if (delta > 2048) {
        delta -= 4096;
    }
    last_wheel = wheel;
}

int16_t get_wheel_delta() {
    return delta;
}
