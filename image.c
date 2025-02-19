
static unsigned char header_data_cmap[][3] = {
	{  0,  0,  0},
	{ 99, 99, 99},
	{120,120,120},
	{126,126,126},
	{147,147,147},
	{182,182,182},
	{255,255,255},
    {0xc0, 0x14, 0x8c},
};

#include <stdio.h>
#include <math.h>

int main() {
    printf("int cmap[] = {");
    for (int i = 0; i < 8; i++) {
        int r = round((float)header_data_cmap[i][0] / 255.0 * 31.0);
        int g = round((float)header_data_cmap[i][1] / 255.0 * 63.0);
        int b = round((float)header_data_cmap[i][2] / 255.0 * 31.0);
        int n = (r << 11) | (g << 5) | b;
        printf("0x%04x", n);
        if (i < 7) printf(", ");
    }
    printf("};\n");
    return 0;
}

// TODO
// Consider double buffering, will either use 44% of the RAM or use a palette. probably see what's best for each use case...
// PIO SPI for background operation?
// Overclocking?
