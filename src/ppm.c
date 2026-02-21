#define ARENA_IMPLEMENTATION

#include "ppm.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


static mem_arena *image_arena;


PPM *ppm_make_image(u32 width, u32 height, Pixel color) {
    image_arena = arena_create(GiB(1));
    // Max 1gb images... Should be enough... right????

    PPM *image = ARENA_PUSH_STRUCT(image_arena, PPM);
    if (!image) return NULL;
    
    image->width = width;
    image->height = height;
    
    image->pixels = (Pixel**)ARENA_PUSH_ARRAY(image_arena, Pixel, height);
    if (!image->pixels) {
        return NULL;
    }
    
    for (u32 y = 0; y < height; y++) {
        image->pixels[y] = ARENA_PUSH_ARRAY(image_arena, Pixel, width);
        if (!image->pixels[y]) {
            for (u32 k = 0; k < y; k++) free(image->pixels[k]);
            return NULL;
        }
        for (u32 x = 0; x < width; x++) {
            image->pixels[y][x] = color;
        }
    }
    return image;
}

void ppm_set_pixel(PPM *image, u32 x, u32 y, Pixel p){
    if(!image || x >= image->width || y >= image->height) return;
    image->pixels[y][x] = p;
}

Pixel ppm_get_pixel(PPM *image, u32 x, u32 y){
    if(!image || x >= image->width || y >= image->height)
        return (Pixel){0,0,0};
    return image->pixels[y][x];
}

PPM *ppm_read_image(const char *fileName) {
    FILE *file = fopen(fileName, "rb");
    if (!file) return NULL;
    
    char format[3];
    if (fscanf(file, "%2s", format) != 1) { fclose(file); return NULL; }
    if (format[0] != 'P' || (format[1] != '3' 
                             && format[1] != '6')) { fclose(file); return NULL; }
    
    int c;
    // Skip comments
    while ((c = fgetc(file)) != EOF) {
        if (c == '#') { while ((c = fgetc(file)) != EOF && c != '\n'); }
        else if (!isspace(c)) { ungetc(c, file); break; }
    }
    
    u32 width, height, maxval;
    if (fscanf(file, "%u %u %u", &width, &height, &maxval) != 3) { fclose(file); return NULL; }
    if (maxval != 255) { fclose(file); return NULL; }
    
    // Skip single whitespace/newline after header
    fgetc(file);
    
    Pixel color = {0,0,0};
    PPM *image = ppm_make_image(width, height, color);
    if (!image) { fclose(file); return NULL; }
    
    if (format[1] == '3') {
        // P3 ASCII
        for (u32 y=0; y<height; y++){
            for (u32 x=0; x<width; x++){
                unsigned int r,g,b;
                if (fscanf(file,"%u %u %u",&r,&g,&b)!=3){
                    ppm_free(image); fclose(file); return NULL;
                }
                image->pixels[y][x].r = r;
                image->pixels[y][x].g = g;
                image->pixels[y][x].b = b;
            }
        }
    } else {
        // P6 Binary
        for (u32 y=0; y<height; y++){
            for (u32 x=0; x<width; x++){
                unsigned char rgb[3];
                if (fread(rgb, 1, 3, file) != 3) {
                    ppm_free(image); fclose(file); return NULL;
                }
                image->pixels[y][x].r = rgb[0];
                image->pixels[y][x].g = rgb[1];
                image->pixels[y][x].b = rgb[2];
            }
        }
    }
    
    fclose(file);
    return image;
}

void ppm_write_image(PPM *image, char* fileName, int binary){
    if(!image) return;
    FILE *file = fopen(fileName, binary ? "wb" : "w");
    if(!file) return;
    
    if (binary)
        fprintf(file, "P6\n%u %u\n255\n", image->width, image->height);
    else
        fprintf(file, "P3\n%u %u\n255\n", image->width, image->height);
    
    for (u32 y=0; y<image->height; y++){
        for (u32 x=0; x<image->width; x++){
            Pixel p = image->pixels[y][x];
            if (binary){
                unsigned char rgb[3] = { (unsigned char)p.r, (unsigned char)p.g, (unsigned char)p.b };
                fwrite(rgb,1,3,file);
            } else {
                fprintf(file,"%u %u %u ", p.r,p.g,p.b);
            }
        }
        if (!binary) fprintf(file,"\n");
    }
    
    fclose(file);
}

void ppm_free() {
    arena_destroy(image_arena);
}