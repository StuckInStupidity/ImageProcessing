#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "detection.h"

#define DIMENSION 28
#define BOX 20
#define AREA 2000
#define NB_PIXELS 20
#define ROW_TOLERANCE 15
#define GRID_TOLERANCE 35
#define CAP 64
#define LEN 64

typedef struct {
	SDL_Surface *s;
	int cx;
	int cy;
	int row;
} L;

static int clamp(int v, int lo, int hi){
	if(v < lo) return lo;
	if(v > hi) return hi;
	return v;
}

static SDL_Surface *normalize(SDL_Surface *src){
	SDL_Surface *n = SDL_CreateRGBSurfaceWithFormat(0, DIMENSION, DIMENSION, 32, PF);
	SDL_FillRect(n, NULL, 0xFFFFFFFF);  
	Uint32 *pixels = (Uint32 *)src->pixels;
	Uint32 *pixou = (Uint32 *)n->pixels;
	double scale = (double)BOX / (src->w > src->h ? src->w : src->h);
	int nw = (int)(src->w * scale + 0.5); if(nw < 1) nw = 1;
	int nh = (int)(src->h * scale + 0.5); if(nh < 1) nh = 1;
	int nx = (DIMENSION - nw) / 2;
	int ny = (DIMENSION - nh) / 2;
	for(int dy = 0; dy < nh; dy++){
		for(int dx = 0; dx < nw; dx++){
			double sx = (dx + 0.5) * src->w / nw - 0.5;
			double sy = (dy + 0.5) * src->h / nh - 0.5;
			int x0 = (int)floor(sx);
			int y0 = (int)floor(sy);
			double fx = sx - x0;
			double fy = sy - y0;
			int x1 = clamp(x0+1, 0, src->w-1);
			int y1 = clamp(y0+1, 0, src->h-1);
			x0 = clamp(x0, 0, src->w-1);
			y0 = clamp(y0, 0, src->h-1);
			double mean = 
			(1-fx)*(1-fy)*(pixels[y0*src->w + x0] & 0xFF)
			+ fx*(1-fy)*(pixels[y0*src->w + x1] & 0xFF)
			+ (1-fx)*fy*(pixels[y1*src->w + x0] & 0xFF)
			+ fx*fy*(pixels[y1*src->w + x1] & 0xFF);
			Uint8 c = (Uint8)(mean + 0.5);
			pixou[(ny + dy)*DIMENSION + (nx + dx)] = (0xFFu<<24)|((Uint32)c<<16)|((Uint32)c<<8)|c;
		}
	}
	return n;
}

void free_letters(Letters *l){
	for(int i=0; i<l->n; i++) SDL_FreeSurface(l->inputs[i]);
	free(l->inputs);
	free(l->row_len);
	free(l->cx);
	free(l->cy);
}

static int cmp_cx(const void *a, const void *b){
	int x = ((const L *)a)->cx;
	int y = ((const L *)b)->cx;
	return (x > y) - (x < y);
}

Letters find_letters(SDL_Surface *image, SDL_Surface *copy, const char *folder, SDL_Rect zone){
	Letters letters = {NULL, NULL, 0};
	int max_h = zone.y + zone.h;
	int max_w = zone.x + zone.w;
	Uint32 *pixels = (Uint32 *)image->pixels;
	int cap = CAP; int cnt=0;
	int row=0, start=0, prev_cy=0, n=0;
	L *arr = malloc(cap * sizeof(L));
	int (*queue)[2] = malloc(sizeof(int[2]) * zone.w * zone.h);
	for(int i=zone.y; i < max_h; i++){
		for(int j=zone.x; j < max_w; j++){
			if(pixels[i*image->w +j] & 0xFF) continue; //not black
			pixels[i*image->w + j] = 0xFFFFFFFF; //white
			int top_left_x = j;
			int top_left_y = i;
			int bottom_right_x = j;
			int bottom_right_y = i;
			int first=0, last=0;
			queue[first][0] = i;
			queue[first][1] = j;
			last++;
			while(first < last){
				int y = queue[first][0];
				int x = queue[first][1];
				first++;
				if(x < top_left_x) top_left_x = x;
				if(x > bottom_right_x) bottom_right_x = x;
				if(y < top_left_y) top_left_y = y;
				if(y > bottom_right_y) bottom_right_y = y;
				for(int dr=-1; dr<=1; dr++){
					for(int dc=-1; dc<=1; dc++){
						int ni = y+dr;
						int nj = x+dc;
						if(ni < zone.y || ni >= max_h || nj < zone.x || nj >= max_w) continue;
						if(!(pixels[ni*image->w +nj] & 0xFF)){
							pixels[ni*image->w +nj] = 0xFFFFFFFF; //white
							queue[last][0] = ni;
							queue[last][1] = nj;
							last++;
						}
					}
				}
			}
			int blob_w = bottom_right_x - top_left_x + 1;
			int blob_h = bottom_right_y - top_left_y + 1;
			if(blob_w*blob_h > NB_PIXELS && blob_w*blob_h < AREA){
				SDL_Rect rect = {top_left_x, top_left_y, blob_w, blob_h};
				SDL_Surface *letter = SDL_CreateRGBSurfaceWithFormat(0,blob_w,blob_h,32,PF);
				SDL_BlitSurface(copy,&rect,letter,NULL);
				if(n == cap) {cap *= 2; arr = realloc(arr, cap*sizeof(L));}
				int cx = (top_left_x + bottom_right_x) / 2;
				int cy = (top_left_y + bottom_right_y) / 2;
				if(n > 0 && (cy - prev_cy > ROW_TOLERANCE)){
					qsort(arr + start, n - start, sizeof(L), cmp_cx);
					row++;
					start = n;
				}
				prev_cy = cy;
				arr[n].s = normalize(letter);
				arr[n].cx = cx;
				arr[n].cy = cy;
				arr[n].row = row;
				if(folder){ //to avoid autorotate using it again
					char name[LEN];
					snprintf(name, sizeof(name), "%s/letter_%d.png", folder, ++cnt);
					IMG_SavePNG(arr[n].s, name);
				}
				SDL_FreeSurface(letter);
				n++;
			}
		}
	}
	qsort(arr + start, n - start, sizeof(L), cmp_cx);
	letters.nb_rows = row + 1;
	letters.inputs = malloc(n * sizeof(SDL_Surface *));
	letters.row_len = calloc(letters.nb_rows, sizeof(int));
	letters.n = n;
	letters.cx = malloc(n * sizeof(int));
	letters.cy = malloc(n * sizeof(int));
	for(int i=0;i<n;i++){
		letters.inputs[i] = arr[i].s;
		letters.row_len[arr[i].row]++;
		letters.cx[i] = arr[i].cx;  //For autorotation only
		letters.cy[i] = arr[i].cy;  //For autorotation only
	}
	if(folder) {
		printf("nb_rows : %d\n", letters.nb_rows); //to avoid autorotate using it again
		for(int i=0; i < letters.nb_rows; i++){
			printf("nb lettres : %d\n", letters.row_len[i]);
		}
		printf("\n");
	}
	free(queue);
	free(arr);
	return letters; //returns local letters makes a copy of the values of the struct
}

static int max(int x, int y){
	return x > y;
}

void detection(SDL_Surface *image, Letters *g, Letters *w){
	ull histo_col[image->w];
	ull histo_row[image->h];
	for(int i=0;i<image->w;i++) histo_col[i]=0;
	for(int i=0;i<image->h;i++) histo_row[i]=0;
	Uint32 *pixels = (Uint32 *)image->pixels;
	for(int i=0; i<image->h; i++){
		for(int j=0; j<image->w; j++){
			Uint32 pixel = pixels[i*image->w +j];
			Uint8 I = (Uint8)(pixel & 0xFF); // since R=G=B=Grey=Black|White
			if(I==0){
				histo_col[j]++;
				histo_row[i]++;
			}
		}
	}

	int top_left_x = (image->w - 1) / 2;
	int top_left_y = (image->h - 1) / 2;
	int bottom_right_x = (image->w - 1) / 2;
	int bottom_right_y = (image->h - 1) / 2;

	int gap=0;
	while(top_left_x > 0 && gap <= GRID_TOLERANCE){
        	top_left_x--;
		if(histo_col[top_left_x] > 1) gap=0;
		else gap++;
	}

	gap=0;
	while(bottom_right_x < image->w - 1 && gap <= GRID_TOLERANCE){
		bottom_right_x++;
		if(histo_col[bottom_right_x] > 1) gap=0;
		else gap++;
	}
	
	gap=0;
	while(top_left_y > 0 && gap <= GRID_TOLERANCE){
        	top_left_y--;
		if(histo_row[top_left_y] > 1) gap=0;
		else gap++;
	}

	gap=0;
	while(bottom_right_y < image->h - 1 && gap <= GRID_TOLERANCE){
		bottom_right_y++;
		if(histo_row[bottom_right_y] > 1) gap=0;
		else gap++;
	}

	int rect_x; int rect_y;
	int rect_w; int rect_h;

	if(max(top_left_x, image->w - bottom_right_x)){
		rect_x = 0;
		rect_y = 0;
		rect_w = top_left_x;
		rect_h = image->h;
	}
	else {
		rect_x = bottom_right_x;
		rect_y = 0;
		rect_w = image->w - bottom_right_x;
		rect_h = image->h;
	}
	if(max(top_left_y, rect_w)){
		rect_x = 0;
		rect_y = 0;
		rect_w = image->w;
		rect_h = top_left_y;
		if(max(image->h - bottom_right_y,rect_h)){
			rect_y = bottom_right_y;
			rect_h = image->h - bottom_right_y;
		}
	}
	if(max(image->h - bottom_right_y,rect_h)){
		rect_x = 0;
		rect_y = bottom_right_y;
		rect_w = image->w;
		rect_h = image->h - bottom_right_y;
	}

	SDL_Rect grid = {top_left_x, top_left_y, bottom_right_x - top_left_x, bottom_right_y - top_left_y};
	SDL_Rect words = {rect_x, rect_y, rect_w, rect_h};
	SDL_Surface *gridpic = SDL_CreateRGBSurfaceWithFormat(0,grid.w,grid.h,32,PF);
	SDL_Surface *wordlist = SDL_CreateRGBSurfaceWithFormat(0,words.w,words.h,32,PF);
	SDL_BlitSurface(image, &grid, gridpic, NULL);
	SDL_BlitSurface(image, &words, wordlist, NULL);
	IMG_SavePNG(gridpic, "output/grid.png");
	IMG_SavePNG(wordlist, "output/wordlist.png");

	SDL_Surface *copy = SDL_CreateRGBSurfaceWithFormat(0,image->w,image->h,32,PF);
	SDL_BlitSurface(image, NULL, copy, NULL);

	*g = find_letters(image, copy, "output/grid", grid); //returns local letters makes a copy of the values of the struct
	*w = find_letters(image, copy, "output/wordlist", words); //returns local letters makes a copy of the values of the struct

	SDL_FreeSurface(copy);
	SDL_FreeSurface(gridpic);
	SDL_FreeSurface(wordlist);
}
