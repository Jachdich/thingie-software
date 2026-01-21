#include <pthread.h>
#include "../include/pico/time.h"
void *pthread_entry(void *arg) {
    void (*entry)(void) = arg;
    entry();
    return NULL;
}

void multicore_launch_core1(void(*entry)(void)) {
     pthread_t thread1;
     pthread_create(&thread1, NULL, pthread_entry, entry);
 }

 long o = 0;
 void multicore_fifo_pop_blocking() {}
 void multicore_fifo_push_blocking() {
     long taken = to_us_since_boot(get_absolute_time()) - o;
     long wait = (1000000 / 30) - taken;
     if (wait > 0) {
         sleep_us(wait - 5);
     }
     o = to_us_since_boot(get_absolute_time());
 }
