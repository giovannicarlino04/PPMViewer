#include <stdio.h>
#include "../../Arena/include/arena.h"

typedef struct{
    u32 r,g,b;
} Pixel;

typedef struct{
    u32  width;
    u32  height;
    Pixel  **pixels;
} PPM;

PPM *ppm_make_image(u32 width, u32 height, Pixel color);
void ppm_set_pixel(PPM *image, u32 x, u32 y, Pixel p);
Pixel ppm_get_pixel(PPM *image, u32 x, u32 y);
PPM *ppm_read_image(const char *fileName);
void ppm_write_image(PPM *image, char* fileName, int binary);
void ppm_free();