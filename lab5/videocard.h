#pragma once
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include <minix/driver.h>
#include <sys/mman.h>

#include <math.h>



int mode_setup(uint16_t mode);

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

int Fillpixel(uint16_t x, uint16_t y, uint32_t color);

void memory_map();

int (vg_draw_rectangle) (uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

void* (vg_init)(uint16_t mode);

void color_setup(uint32_t *color, unsigned row, unsigned column, uint8_t no_rectangles, uint32_t first, uint8_t step);

uint8_t decompose_color(uint32_t color, unsigned mask_size, unsigned field_position);

uint16_t getHres();
uint16_t getVres();


