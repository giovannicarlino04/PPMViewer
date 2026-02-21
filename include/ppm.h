#include <stdio.h>
#include <stdint.h>

typedef struct{
    uint32_t r,g,b;
} Pixel;

typedef struct{
    uint32_t  width;
    uint32_t  height;
    Pixel  **pixels;

} PPM;

PPM *ppm_make_image(uint32_t width, uint32_t height, Pixel color);
void ppm_set_pixel(PPM *image, uint32_t x, uint32_t y, Pixel p);
Pixel ppm_get_pixel(PPM *image, uint32_t x, uint32_t y);
PPM *ppm_read_image(const char *fileName);
void ppm_write_image(PPM *image, char* fileName, int binary);
void ppm_free(PPM *image);