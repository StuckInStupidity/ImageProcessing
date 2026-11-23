#include <stdio.h>
#include <stdlib.h>
#include "processing.h"

#define SIZE 3
#define KERNEL (SIZE*SIZE)
#define RANGE (SIZE >> 1) // => SIZE/2 for unsigned integers
//we compute from var SIZE to avoid dealing with float like #define RANGE (sqrt(KERNEL)/2)
//and to change only one var if we want to modify the kernel

static double mean_or_sum(ull histo[256], int first, int last, int just_sum){
	double tot=0.0, cnt=0.0;
	for(int i=first; i<=last; i++){
		cnt+=histo[i];
		tot+=(double)i*histo[i];
	}
	if(just_sum) return cnt;
	else return cnt!=0.0? (tot/cnt) : cnt;
}

void binarising(SDL_Surface *image){
	ull histo[256]={0};
	Uint32 *pixels = (Uint32 *)image->pixels;
	for(int i=0; i<image->h;i++){
		for(int j=0;j<image->w;j++){
			Uint32 pixel = pixels[i*image->w + j];
			Uint8 I = (Uint8)(pixel & 0xFF); //since r=g=b=grey
			histo[I]++;
		}
	}
	double weighted = 1.0/((double)image->h * image->w);
	Uint8 threshold = 0; double IF = -1.0;
	for(int t=0; t<=255;t++){
		double sum1 = weighted * mean_or_sum(histo,0,t,1);
		double sum2 = weighted * mean_or_sum(histo,t+1,255,1);
		double mean = mean_or_sum(histo,0,t,0);
		double mean2 = mean_or_sum(histo,t+1,255,0);
		double sigma = sum1*sum2*(mean-mean2)*(mean-mean2);
		if(sigma>IF) {IF = sigma; threshold=(Uint8)t;}
	} 
	for(int i=0; i<image->h;i++){
		for(int j=0;j<image->w;j++){
			Uint32 pixel = pixels[i*image->w + j];
			Uint8 I = (Uint8)(pixel & 0xFF); //since r=g=b=grey
			I = I <= threshold ? 0 : 255;
			pixel=(0xFFu<<24)|((Uint32)I<<16)|((Uint32)I<<8)|(Uint32)I;
			pixels[i*image->w + j] = pixel;
		}
	}
	printf("threshold detected : %u\n", threshold);
}

static int comp(const void *a, const void *b){
	Uint8 x = *(const Uint8 *)a;
	Uint8 y = *(const Uint8 *)b;
	return (x > y) - (x < y);
}

SDL_Surface *filtering(SDL_Surface *image){
	SDL_Surface *ans = SDL_CreateRGBSurfaceWithFormat(0,image->w,image->h,32,PF);
	Uint32 *pixels = (Uint32 *)image->pixels;
	Uint32 *pixou = (Uint32 *)ans->pixels;
	Uint8 kernel[KERNEL];
	for(int i=0;i<image->h;i++){
		for(int j=0;j<image->w;j++){
			pixou[i*image->w + j]=pixels[i*image->w + j];
		}
	}
	for(int i=RANGE; i<image->h-RANGE;i++){
		for(int j=RANGE;j<image->w-RANGE;j++){
			Uint8 k=0; Uint32 pixel;
			for(int a=i-RANGE;a<=i+RANGE;a++){
				for(int b=j-RANGE;b<=j+RANGE;b++){
					pixel = pixels[a*image->w + b];
					kernel[k] = (Uint8)(pixel & 0xFF); //since r=g=b=grey
					k++;
				}
			}
			qsort(kernel,KERNEL,sizeof(Uint8),comp);
			Uint8 median = kernel[KERNEL>>1]; //KERNEL/2
			pixel=(0xFFu<<24)|((Uint32)median<<16)|((Uint32)median<<8)|(Uint32)median;
			pixou[i*image->w + j] = pixel;
		}
	}
	return ans;
}

void greyscaling(SDL_Surface *image){
	Uint32 *pixels = (Uint32 *)image->pixels;
	Uint8 min=255,max=0;
	for(int i=0; i<image->h;i++){
		for(int j=0;j<image->w;j++){
			Uint32 pixel = pixels[i*image->w + j];
			Uint8 r = (pixel >> 16) & 0xFF;
			Uint8 g = (pixel >> 8)  & 0xFF;
			Uint8 b = pixel & 0xFF;
			Uint8 grey = (Uint8)(0.3*r+0.59*g+0.11*b);
			if(grey>max) max=grey;
			if(grey<min) min=grey;
		}
	}
	for(int i=0; i<image->h;i++){
		for(int j=0;j<image->w;j++){
			Uint32 pixel = pixels[i*image->w + j];
			Uint8 r = (pixel >> 16) & 0xFF;
			Uint8 g = (pixel >> 8)  & 0xFF;
			Uint8 b = pixel & 0xFF;
			Uint8 grey = (Uint8)(0.3*r+0.59*g+0.11*b);
			if(max>min) grey = (Uint8)(((int)grey-min)*255/(max-min));
			pixel = (0xFFu<<24)|((Uint32)grey<<16)|((Uint32)grey<<8)|(Uint32)grey;
			pixels[i*image->w + j] = pixel;
		}
	}
}