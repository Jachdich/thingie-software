#include <pthread.h>
#include "../include/_time_internal.h"
int main_(void);

int main() {
    pthread_t ptid; 
    pthread_create(&ptid, NULL, timer_update, NULL);
    main_();
    return 0;
}
