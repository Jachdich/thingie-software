#ifndef _MULTICORE_H
#define _MULTICORE_H
void multicore_launch_core1(void(*entry)(void));
int multicore_fifo_pop_blocking();
void multicore_fifo_push_blocking(int);
#endif
