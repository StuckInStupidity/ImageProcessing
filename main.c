#include "misc.h"
#include "processing.h"
#include "rotate.h"
#include "detection.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

double now_ms(){
    return (double)SDL_GetPerformanceCounter()*1000.0/SDL_GetPerformanceFrequency();
}

void help(){
	printf("\nUsage : main.exe image.png predica\n");
	printf("\tpredica : with -> apply median filter\n");
	printf("\tpredica is OPTIONAL\n\n");
}

int main(int argc, char *argv[]){

	help();

	if(argc<2 || argc>3){
		fprintf(stderr, "******************************\n");
		fprintf(stderr, "         Error : args         \n");
		fprintf(stderr, "******************************\n\n");
		return 1;
	}

	int formats = IMG_INIT_PNG | IMG_INIT_JPG;
	int flags = IMG_Init(formats);
	if(!(flags & formats)){
		fprintf(stderr, "%s\n", IMG_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Surface *img = IMG_Load(argv[1]);
	if(!img){
		fprintf(stderr, "%s\n", IMG_GetError());
		IMG_Quit();
		SDL_Quit();
		return 1;
	}
	SDL_Surface *f = img;
	img = SDL_ConvertSurfaceFormat(img, PF, 0);
	SDL_FreeSurface(f);

	greyscaling(img);
	IMG_SavePNG(img, "output/greyscaled.png");

	SDL_Surface *new_img = img;
	if(argc==3 && strcmp(argv[2],"with")==0) {
		new_img = filtering(img);
		IMG_SavePNG(new_img, "output/filtered.png");
		SDL_FreeSurface(img);
	}

	binarising(new_img);
	IMG_SavePNG(new_img, "output/binarised.png");

	SDL_Surface *latest = new_img;
	double angle = autoRotate(new_img);
	printf("angle detected : %lf\n\n", angle);
	if(fabs(angle) > 0.5){
		latest = manRotate(new_img, angle);
		IMG_SavePNG(latest, "output/rotated.png");
		SDL_FreeSurface(new_img);
	}

	Letters g = {NULL, NULL, 0};
	Letters w = {NULL, NULL, 0};

	//double cnt0 = now_ms();
	detection(latest, &g, &w);
	//double cnt1 = now_ms();
	//printf("profiling : %.2f ms\n", cnt1 - cnt0);

	free_letters(&g);
	free_letters(&w);
	SDL_FreeSurface(latest);
	IMG_Quit();
	SDL_Quit();
	return 0;
}

