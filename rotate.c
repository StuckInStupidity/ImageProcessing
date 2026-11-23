#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "detection.h"
#include "rotate.h"

#define BIN 5

SDL_Surface *manRotate(SDL_Surface *image, double angle){
	SDL_Surface *ans = SDL_CreateRGBSurfaceWithFormat(0,image->w,image->h,32,PF);
	Uint32 *pixels = (Uint32 *)image->pixels;
	Uint32 *pixou = (Uint32 *)ans->pixels;
	double radian = angle*M_PI/180.0;
	double c = cos(radian);
	double s = sin(radian);
	double cx = (image->w-1.0)/2.0, cy = (image->h-1.0)/2.0;
	for(int i=0;i<image->h;i++){
		for(int j=0;j<image->w;j++){
			double dx=j-cx, dy=i-cy;
			int py=(int)round(cy+dy*c+dx*s);
			int px=(int)round(cx-dy*s+dx*c);
			if(py<0||py>=image->h||px<0||px>=image->w) pixou[i*image->w+j]=0xFFFFFFFF;
			else pixou[i*image->w+j]=pixels[py*image->w+px];
		}
	}
	return ans;
}

double autoRotate(SDL_Surface *image){
	SDL_Surface *detecting = SDL_CreateRGBSurfaceWithFormat(0,image->w,image->h,32,PF);
	SDL_BlitSurface(image, NULL, detecting, NULL);
	SDL_Rect all = {0, 0, image->w, image->h};
	Letters l = find_letters(detecting, image, NULL, all);
	int size = (image->h+2*image->w)/BIN +2;
	int *bins = malloc(size*sizeof(int));
	double best = 0; ull best_cnt = 0;
	for(double angle = -45; angle <= 45; angle += 0.5){
		double c = cos(angle*M_PI/180);
		double s = sin(angle*M_PI/180);
		memset(bins,0,size*sizeof(int));
		for(int i=0; i<l.n; i++) {
			int index = (int)((l.cy[i]*c - l.cx[i]*s + image->w) / BIN);
			bins[index]++;
		}
		ull cnt = 0;
		for(int i=0; i<size; i++) cnt+=(ull)bins[i]*bins[i];
		if(cnt > best_cnt){
			best_cnt = cnt;
			best = angle;
		}
	}
	free(bins);free_letters(&l);SDL_FreeSurface(detecting);
	return best;
}
