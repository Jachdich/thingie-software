#include <pthread.h>
#include "../include/_time_internal.h"
#include "../../include/drawing.h"
int main_(void);
void init(void);

int music_step(void *a, Screen s) {
    return 0;
}
void music_init(void *a) {
    
}

int main() {
    pthread_t ptid; 
    pthread_create(&ptid, NULL, timer_update, NULL);
    init();
    main_();
    return 0;
}
