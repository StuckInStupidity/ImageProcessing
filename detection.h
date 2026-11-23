#ifndef DETECTION_H
#define DETECTION_H

#include "misc.h"

typedef struct {
	SDL_Surface **inputs;
	int *row_len;
	int nb_rows;
	int n;
	int *cx;       //For autorotation only
	int *cy;       //For autorotation only
} Letters;

void detection(SDL_Surface *image, Letters *g, Letters *w);
Letters find_letters(SDL_Surface *image, SDL_Surface *copy, const char *folder, SDL_Rect zone);
void free_letters(Letters *l);

#endif