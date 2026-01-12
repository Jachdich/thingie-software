#include <pthread.h>
void *pthread_entry(void *arg) {
    void (*entry)(void) = arg;
    entry();
    return NULL;
}

void multicore_launch_core1(void(*entry)(void)) {
     pthread_t thread1;
     pthread_create(&thread1, NULL, pthread_entry, entry);
 }
 void multicore_fifo_pop_blocking() {}
 void multicore_fifo_push_blocking() {}
