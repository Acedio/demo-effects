#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "pixel.h"

#define SCRW 320
#define SCRH 240
#define SCRBPP 24

#define PI 3.14159

#define WAVECOUNT 15

void twist(SDL_Surface *dst, float *amps, float *freqs, int wavecount, int t){
	int y;
	for(y = 0; y < dst->h; y++){
		int i;
		float theta = 0;
		for(i = 0; i < wavecount; i++){
			theta += amps[i]*sin((float)(y+t)*freqs[i]/1000);
		}
		while(theta > PI){
			theta -= PI;
		}
		while(theta < 0){
			theta += PI;
		}
		float mul = SCRW/3;
		int w;
		if(theta < PI/2){
			w = mul*(sin(theta) + cos(theta));
		} else {
			w = mul*(sin(theta) - cos(theta));
		}
		int x;
		int stop = x = (SCRW - w)/2;
		if(theta < PI/2){
			int iw = mul*sin(theta);
			stop += iw;
			for(; x < stop; x++){
				putpixel(dst,x,y,((0xFF*iw/w)<<8)+(0xFF*iw/w));
			}
			iw = mul*cos(theta);
			stop += iw;
			for(; x < stop; x++){
				putpixel(dst,x,y,((0xFF*iw/w)<<16)+(0xFF*iw/w));
			}
		} else {
			int iw = mul*-cos(theta);
			stop += iw;
			for(; x < stop; x++){
				putpixel(dst,x,y,((0xFF*iw/w)<<16)+(0xFF*iw/w));
			}
			iw = mul*sin(theta);
			stop += iw;
			for(; x < stop; x++){
				putpixel(dst,x,y,((0xFF*iw/w)<<8)+(0xFF*iw/w));
			}
		}
	}
}

Uint32 fire_pal(int i){
	int divs = 4;
	if(i < 0x40){
		int l = i*4;
		return (l<<8) + (l<<0);
	} else if(i < 0x80){
		i -= 0x40;
		return (0xFFFF - ((i*2)<<8))<<0;
	} else if(i < 0xC0){
		i -= 0x80;
		return (0xFF7F - ((i*2)<<8))<<0;
	} else { // i >= 0xC0
		return 0x00FF00;
	}
}

void fire(SDL_Surface *dst, int **grid, int fw, int fh, float *amps, float *freqs, int wavecount, int t){
	int y;
	int x;
	int bx = 20*(cos(freqs[0]*t/100) + cos(freqs[1]*t/100) + 2);
	int by = 30*(sin(freqs[2]*t/100) + sin(freqs[3]*t/100) + 2);
	for(y = 0; y < 3; y++){
		for(x = 0; x < 3; x++){
			grid[y+by][x+bx] = 0xFF;
			grid[y+by][SCRW-1-(x+bx)] = 0xFF;
		}
	}
	for(y = 0; y < fh - 1; y++){
		for(x = 0; x < fw; x++){
			int val;
			if(x <= 0){
				val = ((grid[y][x]) + (grid[y+1][x+1] - 1) + (grid[y+1][x] - 1)*7)/10;
			} else if(x >= fw - 1){
				val = ((grid[y][x]) + (grid[y+1][x-1] - 1) + (grid[y+1][x] - 1)*7)/10;
			} else {
				val = ((grid[y][x]) + (grid[y+1][x-1] - 1) + (grid[y+1][x+1] - 1) + (grid[y+1][x] - 1)*7)/10;
			}
			grid[y][x] = val > 0 ? val : 0;
			putpixel(dst,x,y,fire_pal(grid[y][x]));
		}
	}
	for(x = 0; x < fw; x++){
		int i;
		float theta = 0;
		for(i = 0; i < wavecount; i++){
			theta += amps[i]*sin((float)(x+t*10)*freqs[i]/10);
		}
		while(theta > PI){
			theta -= PI;
		}
		while(theta < 0){
			theta += PI;
		}
		float mul = SCRW/3;
		grid[fh-1][x] = mul*theta;
		putpixel(dst,x,fh-1,fire_pal(grid[fh-1][x]));
	}
}

int main(){
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Surface* screen = SDL_SetVideoMode(SCRW,SCRH,SCRBPP,SDL_SWSURFACE);

	SDL_Event event;

	int running = 1;

	Uint32 start = SDL_GetTicks();
	Uint32 fpstime = start;
	int frames = 0;
	int FPS = 60;
	int MSPF = 1000/FPS;

	srand(time(0));

	int i;
	float amps[WAVECOUNT], freqs[WAVECOUNT];
	for(i = 0; i < WAVECOUNT; i++){
		amps[i] = (float)((rand()>>4)%50)/10.0;
		freqs[i] = 4.0/(float)(1+((rand()>>4)%50));
		printf("amp: %f   freq: %f\n",amps[i],freqs[i]);
	}

	int **grid = (int**)malloc(SCRH*sizeof(int*));
	for(i = 0; i < SCRH; i++){
		grid[i] = (int*)malloc(SCRW*sizeof(int));
		int x;
		for(x = 0; x < SCRW; x++){
			grid[i][x] = 0;
		}
	}

	while(running){
		Uint8* keystates = SDL_GetKeyState(NULL);
		if(keystates[SDLK_q]){
			running = 0;
		}
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				running = 0;
			}
		}
		SDL_Rect r;
		r.x = r.y = 0;
		r.w = SCRW;
		r.h = SCRH;
		SDL_FillRect(screen,&r,0);
		fire(screen, grid, SCRW, SCRH, amps, freqs, WAVECOUNT, frames*10);
		twist(screen, amps, freqs, WAVECOUNT, frames*10);
		SDL_Flip(screen);
		frames++;
		if(frames%100 == 0){
			//printf("%i\n",frames*1000/(SDL_GetTicks()-fpstime));
		}
		Uint32 elapsed = SDL_GetTicks() - start;
		if(elapsed < MSPF){
			SDL_Delay(MSPF - elapsed);
		}
		start = SDL_GetTicks();
	}

	SDL_FreeSurface(screen);
	SDL_Quit();
	return 0;
}
