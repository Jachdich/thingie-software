#include "../include/minesweeper.h"
#include "../include/drawing.h"
#include "../include/keypad.h"
// #include "../include/st7789.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "pico/time.h"

//TODO
//chording
//main menu
//better scaling
//num of mines
//mines left display
//time
//fix annoying "clever" code

#define MINE 0x0F
#define FLAGGED (1<<6)
#define REVEALED (1<<7)

const uint16_t neighbour_colours[] = {
    0b0000000000000000,
    0b0000000000011111,
    0b0000010110000000,
    0b1111100000000000,
    0b0000100000011000,
    0b1000000000000000,
    0b0000010000010000,
    0b0000000000000000,
    0b1000010000010000,
};

bool big_numbers[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,
    0,0,0,0,1,1,1,0,0,1,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,0,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,
    0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,
    0,0,0,1,1,1,1,0,0,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,0,0,1,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,
    0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,2,1,1,0,0,0,0,0,0,
    0,0,0,0,0,2,2,1,1,0,0,0,0,0,0,
    0,0,0,0,2,2,2,1,1,0,0,0,0,0,0,
    0,0,0,2,2,2,2,1,1,0,0,0,0,0,0,
    0,0,0,2,2,2,2,1,1,0,0,0,0,0,0,
    0,0,2,2,2,2,2,1,1,0,0,0,0,0,0,
    0,0,2,2,2,2,2,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

// uint16_t test_number[] = {
//     0b000000000000000,
//     0b000000000000000,
//     0b000011100000000,
//     0b000011110000000,
//     0b000011111000000,
//     0b000011011100000,
//     0b000011001110000,
//     0b000011000111000,
//     0b000111111111000,
//     0b000111111111000,
//     0b000011000000000,
//     0b000011000000000,
//     0b000011000000000,
//     0b000000000000000,
//     0b000000000000000,};

unsigned char big_revealed_data[] = {
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};
unsigned char big_unrevealed_data[] = {
    2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    2,3,3,3,3,3,3,3,3,3,3,3,3,3,1,
    2,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    2,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    2,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

unsigned char small_revealed_data[] = {
    7,7,7,7,7,7,7,7,7,
    7,6,6,6,6,6,6,6,7,
    7,6,6,6,6,6,6,6,7,
    7,6,6,6,6,6,6,6,7,
    7,6,6,6,6,6,6,6,7,
    7,6,6,6,6,6,6,6,7,
    7,6,6,6,6,6,6,6,7,
    7,6,6,6,6,6,6,6,7,
    7,7,7,7,7,7,7,7,7
};

unsigned char small_unrevealed_data[] = {
    2,2,2,2,2,2,2,2,1,
    2,2,2,2,2,2,2,1,1,
    2,2,3,3,3,3,3,1,1,
    2,2,3,3,3,3,3,1,1,
    2,2,3,3,3,3,3,1,1,
    2,2,3,3,3,3,3,1,1,
    2,2,3,3,3,3,3,1,1,
    2,2,1,1,1,1,1,1,1,
    2,1,1,1,1,1,1,1,1
    };

// TODO bit pack these?
bool small_numbers[] = {
    0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,1,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,0,0,0,1,0,0,
    0,0,0,0,1,1,0,0,0,
    0,0,0,1,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,
    0,0,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,0,0,0,1,0,0,
    0,0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,1,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,
    
    0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,0,0,0,
    0,0,0,1,0,1,0,0,0,
    0,0,1,0,0,1,0,0,0,
    0,0,1,1,1,1,1,0,0,
    0,0,0,0,0,1,0,0,0,
    0,0,0,0,0,1,0,0,0,
    0,0,0,0,0,1,0,0,0,
    0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,0,0,
    0,0,1,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,1,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,0,0,0,
    0,0,0,1,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,
    0,0,1,1,1,1,0,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,1,0,0,
    0,0,0,0,0,0,1,0,0,
    0,0,0,0,0,1,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,0,
    0,0,0,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,1,0,0,0,1,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,
    
    0,0,0,0,0,0,0,0,0,
    0,0,0,2,1,0,0,0,0,
    0,0,2,2,1,0,0,0,0,
    0,2,2,2,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,

};

uint16_t cmap[] = {0x0000, 0x630c, 0x7bcf, 0x7bef, 0x9492, 0xb5b6, 0xffff, 0xdefb};

MaskImage numbers[] = {
    {.data = small_numbers + 9*9*0, .size = {9, 9}},
    {.data = small_numbers + 9*9*1, .size = {9, 9}},
    {.data = small_numbers + 9*9*2, .size = {9, 9}},
    {.data = small_numbers + 9*9*3, .size = {9, 9}},
    {.data = small_numbers + 9*9*4, .size = {9, 9}},
    {.data = small_numbers + 9*9*5, .size = {9, 9}},
    {.data = small_numbers + 9*9*6, .size = {9, 9}},
    {.data = small_numbers + 9*9*7, .size = {9, 9}},
    {.data = small_numbers + 9*9*8, .size = {9, 9}},
    {.data = small_numbers + 9*9*9, .size = {9, 9}},
    {.data = big_numbers + 15*15*0, .size = {15, 15}},
    {.data = big_numbers + 15*15*1, .size = {15, 15}},
    {.data = big_numbers + 15*15*2, .size = {15, 15}},
    {.data = big_numbers + 15*15*3, .size = {15, 15}},
    {.data = big_numbers + 15*15*4, .size = {15, 15}},
    {.data = big_numbers + 15*15*5, .size = {15, 15}},
    {.data = big_numbers + 15*15*6, .size = {15, 15}},
    {.data = big_numbers + 15*15*7, .size = {15, 15}},
    {.data = big_numbers + 15*15*8, .size = {15, 15}},
    {.data = big_numbers + 15*15*9, .size = {15, 15}},
};

PaletteImage small_unrevealed = {.data = small_unrevealed_data, .size = {9,9}, .palette = cmap};
PaletteImage small_revealed   = {.data = small_revealed_data,   .size = {9,9}, .palette = cmap};
PaletteImage big_unrevealed   = {.data = big_unrevealed_data,   .size = {15, 15}, .palette = cmap};
PaletteImage big_revealed     = {.data = big_revealed_data,     .size = {15, 15}, .palette = cmap};


int count_neighbours(MineState *state, int x, int y) {
    int neighbours = 0;
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            int bx = x + dx;
            int by = y + dy;
            if (bx < 0 || bx >= state->width ||
                by < 0 || by >= state->height) {
                continue;
            }
            if (state->board[by * state->width + bx] == MINE) {
                neighbours += 1;
            }
        }
    }
    return neighbours;
}

void generate_board(MineState *state) {
    memset(state->board, 0, 26 * 26 * sizeof(*state->board));
    for (int i = 0; i < state->num_mines; i++) {
        int pos;
        //make sure the chosen pos doesn't already have a mine
        do {
            pos = rand() % (state->width * state->height);
        } while (state->board[pos] == MINE);
        state->board[pos] = MINE;
    }

    //number non-mine squares
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            if (state->board[y * state->width + x] == MINE) {
                continue;
            }
            state->board[y * state->width + x] = count_neighbours(state, x, y);
        }
    }
}

//return whether the game should continue
bool reveal(MineState *state, int x, int y) {
    if (state->board[y * state->width + x] & 0x80) {
        //already revealed
        return true;
    }
    if (state->board[y * state->width + x] & 0x40) {
        //flagged, don't reveal
        return true;
    }
    if (state->board[y * state->width + x] == MINE) {
        //explode
        return false;
    }
    
    state->board[y * state->width + x] |= 0x80;
    if (state->board[y * state->width + x] == 0x80) {
        //0 neighbours, recursively propagate
        for (int dx = -1; dx < 2; dx++) {
            for (int dy = -1; dy < 2; dy++) {
                if (x + dx < 0 || x + dx >= state->width ||
                    y + dy < 0 || y + dy >= state->height) {
                    continue;
                }
                if (state->board[(y + dy) * state->width + x + dx] & 0x80) {
                    continue;
                }
                
                reveal(state, x + dx, y + dy);
            }
        }
    }
    
    return true;
}

void draw_tile(int x, int y, MineState *state, Screen s) {
    uint8_t tile = state->board[y * state->width + x];
    x = x * state->cell_rad;// + 1;
    y = y * state->cell_rad;// + 1;
    
    const uint16_t flag_colours[2] = {
        0, // pole: black
        0b1111100000000000, // flag: red
    };
    
    bool big = state->cell_rad== 15 ? 1 : 0; // TODO hacky

    if (tile & REVEALED) {
        uint8_t neighbours = tile & 0x0F;

        draw_palette(s, big ? big_revealed : small_revealed, (Vec2){x, y});
        if (neighbours > 0) {
            draw_mask(s, numbers[(neighbours - 1) + 10 * big], (Vec2){x, y}, &neighbour_colours[neighbours]);
        }
    } else if (tile & FLAGGED) {
        draw_palette(s, big ? big_unrevealed : small_unrevealed, (Vec2){x, y});
        draw_mask(s, numbers[8 + 10 * big], (Vec2){x, y}, flag_colours);
    } else {
        draw_palette(s, big ? big_unrevealed : small_unrevealed, (Vec2){x, y});
    }
}

void draw_selected(Screen s, int x0, int y0, int w, int h, uint16_t col) {
    draw_xline(s, (Vec2){x0, y0}, w, col);
    draw_xline(s, (Vec2){x0, y0 + h - 1}, w, col);
    if (y0 - 1 >= 0) {
        draw_xline(s, (Vec2){x0, y0 - 1}, w, col);
    }
    if (y0 + h < 240) {
        draw_xline(s, (Vec2){x0, y0 + h}, w, col);
    }

    draw_yline(s, (Vec2){x0, y0}, h, col);
    draw_yline(s, vec2(x0 + w - 1, y0), h, col);

    if (x0 - 1 >= 0) {
        draw_yline(s, vec2(x0 - 1, y0), h, col);
    }
    if (x0 + w < 240) {
        draw_yline(s, vec2(x0 + w, y0), h, col);
    }
}

void draw(MineState *state, Screen s) {
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            draw_tile(x, y, state, s);
            // uint16_t border = state->selected_x == x && state->selected_y == y ? 0b1111111111111111 : 0b1000010000010000;
            // tft->drawRect(x * state->cell_width, y * state->cell_height, state->cell_width, state->cell_height, border);
        }
    }
    draw_selected(s, state->selected_x * state->cell_rad,
                          state->selected_y * state->cell_rad,
                          state->cell_rad, state->cell_rad,
                          0b1111100000000000);
                          // 0xb8b1);
}

bool check_win(MineState *state) {
    int unrevealed = 0;
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            //if the top bit (i.e. revealed) isn't set
            if (!(state->board[y * state->width + x] & REVEALED)) {
                unrevealed += 1;
            }
        }
    }
    return unrevealed == state->num_mines;
}

void minesweeper_init(MineState *state) {
    //semi random ish, read from uninitialised analog pin
    // srand(analogRead(A7) ^ micros());
    srand(to_us_since_boot(get_absolute_time()));
    
    //board: mine = 0x0F, non-mine = n where n is the number of neighbours
    //top bit set if revealed (|= 0x80)
    //2nd-top bit set if flagged (|= 0x40)
    // num_mines = width * height / 6
    int default_width = 16;
    int default_height = 16;
    int default_num_mines = 42;
    int cell_rad = default_width > 16 ? 9 : 15;

    state->selected_x = 0; 
    state->selected_y = 0;
    state->width = default_width;
    state->height = default_height;
    state->cell_rad = cell_rad;
    state->num_mines = default_num_mines;
    state->view = MINEVIEW_INIT_MENU;
}

bool draw_endgame(MineState *state, Screen s, const char *message, const struct mf_font_s *font) {
    bool go = keypad_get(2, 1).pressed;
    if (keypad_get(2, 0).pressed) {
        state->selected_x -= 1;
        if (state->selected_x < 0) state->selected_x = 0;
    }
    if (keypad_get(2, 2).pressed) {
        state->selected_x += 1;
        if (state->selected_x > 1) state->selected_x = 1;
    }
    keypad_next_frame();
    draw_string(s, message, vec2(60, 50), 0xffff, font, MF_ALIGN_LEFT);
    draw_string(s, "Again", vec2(60, 62), 0xffff, font, MF_ALIGN_LEFT);
    draw_string(s, "Quit", vec2(60, 74), 0xffff, font, MF_ALIGN_LEFT);
    draw_string(s, ">", vec2(50, 62 + 12 * state->selected_x), 0xffff, font, MF_ALIGN_LEFT);
    if (state->selected_x == 0 && go) {
        state->view = MINEVIEW_INIT_MENU;
    }
    if (state->selected_x == 1 && go) return false;
    return true;
}

bool minesweeper_step(MineState *state, Screen s) {
    static const struct mf_font_s *font = NULL; // TODO not static....
    if (font == NULL) font = mf_find_font("DejaVuSans12");
    switch (state->view) {
        case MINEVIEW_PLAYING:
            state->selected_x += (keypad_get(3, 1).pressed) && state->selected_x < state->width - 1;
            state->selected_y += (keypad_get(2, 2).pressed) && state->selected_y < state->height - 1;
            state->selected_x -= (keypad_get(1, 1).pressed) && state->selected_x > 0;
            state->selected_y -= (keypad_get(2, 0).pressed) && state->selected_y > 0;
            if (keypad_get(2, 1).released) {
                if (!reveal(state, state->selected_x, state->selected_y)) {
                    state->view = MINEVIEW_GAME_OVER;
                    state->selected_x = 0;
                } else if (check_win(state)) {
                    state->view = MINEVIEW_COMPLETE;
                    state->selected_x = 0;
                }
            }
            if (keypad_get(0, 1).released) {
                //toggle flagging, if unrevealed
                int idx = state->selected_y * state->width + state->selected_x;
                if (!(state->board[idx] & REVEALED)) {
                    state->board[idx] ^= FLAGGED;
                }
            }
            keypad_next_frame();
            draw(state, s);
            break;
        case MINEVIEW_INIT_MENU: {
            int delta = 0;
            bool go = keypad_get(2, 1).pressed;
            if (keypad_get(1, 1).pressed || (keypad_get(1, 1).f_pressed > 30 && keypad_get(1, 1).f_pressed % 4 == 0)) {
                delta = -1;
            }
            if (keypad_get(3, 1).pressed || (keypad_get(3, 1).f_pressed > 30 && keypad_get(3, 1).f_pressed % 4 == 0)) {
                delta = 1;
            }
            if (keypad_get(2, 0).pressed) {
                state->selected_x -= 1;
                if (state->selected_x < 0) state->selected_x = 0;
            }
            if (keypad_get(2, 2).pressed) {
                state->selected_x += 1;
                if (state->selected_x > 3) state->selected_x = 3;
            }
            keypad_next_frame();
            if (state->selected_x == 0) {
                state->width += delta;
                if (delta != 0) {
                    state->num_mines = state->width * state->height / 6;
                }
                if (state->width < 1) state->width = 1;
                if (state->width > 26) state->width = 26;
            }
            if (state->selected_x == 1) {
                state->height += delta;
                if (delta != 0) {
                    state->num_mines = state->width * state->height / 6;
                }
                if (state->height < 1) state->height = 1;
                if (state->height > 26) state->height = 26;
            }
            if (state->selected_x == 2) {
                state->num_mines += delta;
                if (state->num_mines < 0) state->num_mines = 0;
                if (state->num_mines > 999) state->num_mines = 999;
            }

            uint16_t x0 = 50;
            uint16_t y0 = 50;

            if (state->selected_x == 3) {
                if (go) {
                    state->selected_x = 0;
                    generate_board(state);
                    keypad_next_frame(); // stop button pressing going through to the game;
                    state->view = MINEVIEW_PLAYING;
                    state->cell_rad = (state->width > 16 || state->height > 16) ? 9 : 15;
                    break;
                }
                // draw_string(s, "[", vec2(x0, y0 + 36), 0xffff, font, MF_ALIGN_LEFT);
                // draw_string(s, "]", vec2(x0 + 40, y0 + 36), 0xffff, font, MF_ALIGN_LEFT);
            }

            draw_string(s, "Width", vec2(x0 + 10, y0), 0xffff, font, MF_ALIGN_LEFT);
            draw_string(s, "Height", vec2(x0 + 10, y0 + 12), 0xffff, font, MF_ALIGN_LEFT);
            draw_string(s, "Mines", vec2(x0 + 10, y0 + 24), 0xffff, font, MF_ALIGN_LEFT);
            draw_string(s, "Start", vec2(x0 + 10, y0 + 36), 0xffff, font, MF_ALIGN_LEFT);

            char buf[4];
            buf[2] = 0;
            buf[0] = state->width / 10 + '0';
            buf[1] = state->width % 10 + '0';
            draw_string(s, buf, vec2(x0 + 60, y0), 0xffff, font, MF_ALIGN_LEFT);
            buf[0] = state->height / 10 + '0';
            buf[1] = state->height % 10 + '0';
            draw_string(s, buf, vec2(x0 + 60, y0 + 12), 0xffff, font, MF_ALIGN_LEFT);
            buf[0] = state->num_mines / 100 + '0';
            buf[1] = (state->num_mines / 10) % 10 + '0';
            buf[2] = state->num_mines % 10 + '0';
            buf[3] = 0;
            draw_string(s, buf, vec2(x0 + 60, y0 + 24), 0xffff, font, MF_ALIGN_LEFT);

            draw_string(s, ">", vec2(x0, y0 + 12 * state->selected_x), 0xffff, font, MF_ALIGN_LEFT);
            break;
        }
        case MINEVIEW_COMPLETE: return draw_endgame(state, s, "Success", font);
        case MINEVIEW_GAME_OVER:return draw_endgame(state, s, "Game Over", font);
        default:
            keypad_next_frame();
            break;
    }
    return true;
}
