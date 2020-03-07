#pragma once

#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include <minix/driver.h>
#include <sys/mman.h>

typedef struct {
	int x, y; // current position
	int width, height; // dimensions
	int xspeed, yspeed; // current speed
	char* map; // the pixmap
} Sprite;

Sprite* create_sprite(const char* pic[], int x, int y, int xspeed, int yspeed);

void destroy_sprite(Sprite* sp);
