#ifndef _HARDWARE_PIO_H
#define _HARDWARE_PIO_H
typedef struct {
    char a[0x0188];
} PIO;
extern PIO pio0;
extern PIO pio1;
#endif
