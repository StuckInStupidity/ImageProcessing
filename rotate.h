#ifndef ROTATE_H
#define ROTATE_H

#include "misc.h"

SDL_Surface *manRotate(SDL_Surface *image, double angle);
double autoRotate(SDL_Surface *image);

#endif