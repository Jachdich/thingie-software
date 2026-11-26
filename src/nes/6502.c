#include <stdint.h>
#include "../../include/drawing.h"

void loop(Screen s) {
    draw_circle(s, vec2(10, 10), 10, 0xffff);
}
