#ifndef PROCESSING_H
#define PROCESSING_H

#include "misc.h"

void greyscaling(SDL_Surface *image);
SDL_Surface *filtering(SDL_Surface *image);
void binarising(SDL_Surface *image);

#endif