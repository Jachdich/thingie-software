#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>
#include <pthread.h>
#include "pico/time.h"

SDL_Surface *winSurface = NULL;
SDL_Window *window = NULL;

const uint32_t scl = 2;
const uint32_t border = 8;

extern int __test_keypad[12];

void update_keystate(SDL_Keycode code, int state) {
    switch (code) {
        case SDLK_u: __test_keypad[0] = state; break;
        case SDLK_i: __test_keypad[1] = state; break;
        case SDLK_o: __test_keypad[6] = state; break;
        case SDLK_p: __test_keypad[7] = state; break;
        case SDLK_j: __test_keypad[2] = state; break;
        case SDLK_k: __test_keypad[3] = state; break;
        case SDLK_l: __test_keypad[8] = state; break;
        case SDLK_SEMICOLON: __test_keypad[9] = state; break;
        case SDLK_m: __test_keypad[4] = state; break;
        case SDLK_COMMA: __test_keypad[5] = state; break;
        case SDLK_PERIOD: __test_keypad[10] = state; break;
        case SDLK_SLASH: __test_keypad[11] = state; break;
    }
}

uint16_t wheel = 0;

void *draw_thread(void* a) {
    SDL_Event ev;
    while (1) {	
    	while (SDL_PollEvent(&ev) != 0) {
    		switch (ev.type) {
    			case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    window = NULL;
                	SDL_Quit();
                	exit(0);
                case SDL_KEYDOWN:
                    update_keystate(ev.key.keysym.sym, 0); 
                    break;
                case SDL_KEYUP:
                    update_keystate(ev.key.keysym.sym, 1); 
                    break;
                case SDL_MOUSEWHEEL:
                    wheel += ev.wheel.y * 64;
                    break;
    		}
    	}

    	SDL_Rect rects[4] = {
    	    {0, 0, 240 * scl + 2 * border, border},
    	    {0, 0, border, 240 * scl + 2 * border},
    	    {240 * scl + border, 0, border, 240 * scl + border},
    	    {0, 240 * scl + border, 240 * scl + border, border},
    	};
    	for (int i = 0; i < 4; i++) {
        	SDL_FillRect(winSurface, &rects[i], SDL_MapRGB(winSurface->format, 0x07, 0x23, 0x14));
    	}

    	SDL_Delay(10);
        SDL_UpdateWindowSurface(window);
    }
}

void init() {
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return;
	} 

	window = SDL_CreateWindow( "Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240 * scl + 2 * border, 240 * scl + 2 * border, SDL_WINDOW_SHOWN );

	if ( !window ) {
		printf("Error creating windo: %s\n", SDL_GetError());
		return;
	}

	winSurface = SDL_GetWindowSurface( window );

	if ( !winSurface ) {
		printf("Error geting surfa: %s\n", SDL_GetError());
		return;
	}

     pthread_t thread1;
     pthread_create(&thread1, NULL, draw_thread, NULL);

	// End the program
	// return 0;
}

int x = 0;
int y = 0;
int init_byte = 0;
int which_byte = 0;
void st7789_start_pixels(int a, int b) {
    which_byte = 0;
    init_byte = 0;
    x = 0;
    y = 0;
}

void draw_pixel(uint16_t n, int x, int y) {
    uint8_t r = ((n & 0b1111100000000000) >> (6 + 5)) << 3;
    uint8_t g = ((n & 0b0000011111100000) >> 5) << 2;
    uint8_t b = ((n & 0b0000000000011111) >> 0) << 3;
    SDL_Rect rect = {x * scl + border, y * scl + border, scl, scl};
    if (window == NULL) return;
	SDL_FillRect(winSurface, &rect, SDL_MapRGB(winSurface->format, r, g, b));
}

typedef struct {
    char a[0x0188];
} PIO;

void st7789_lcd_put(PIO a, uint b, uint8_t n) {
    if (which_byte == 0) {
        init_byte = n;
        which_byte = 1;
    } else {
        draw_pixel(((uint16_t)init_byte << 8) | n, x, y);
        which_byte = 0;
        x += 1;
        if (x >= 240) {
            x = 0;
            y += 1;
        }
    }
}


uint16_t as5600_read_raw_angle() {
    return wheel;
}

uint16_t as5600_read_adc() {
    return 16384;
}
