#include <math.h>
#include <SDL/SDL.h>
#include <stdio.h>
#include "pixel.h"

#define SCRW 320
#define SCRH 240
#define SCRBPP 24

#define TABLEWIDTH 256
#define PI 3.141592653589

#define PALWIDTH 256

char sine[TABLEWIDTH];
Uint32 pal[PALWIDTH];

void sin_pop(){
	int i;
	for(i = 0; i < TABLEWIDTH; ++i){
		sine[i] = (int)((double)-.5+(double)127.5*sin((double)PI*(double)i/(double)128));
	}
}

void pal_pop(SDL_PixelFormat* fmt){
	int i,p=0;
	for(i = 0; i < PALWIDTH/4; ++i,++p)
		pal[p] = SDL_MapRGB(fmt,255-2*i,127+2*i,0);
	for(i = 0; i < PALWIDTH/4; ++i,++p)
		pal[p] = SDL_MapRGB(fmt,127-2*i,255-2*i,0+2*i);
	for(i = 0; i < PALWIDTH/4; ++i,++p)
		pal[p] = SDL_MapRGB(fmt,0+2*i,127-2*i,127-2*i);
	for(i = 0; i < PALWIDTH/4; ++i,++p)
		pal[p] = SDL_MapRGB(fmt,127+2*i,0+2*i,0);
}


int main(){
	sin_pop();
	int running = 1;
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Surface* screen = SDL_SetVideoMode(SCRW, SCRH, SCRBPP, SDL_SWSURFACE | SDL_FULLSCREEN);
	pal_pop(screen->format);
	SDL_Event event;
	Uint32 start = SDL_GetTicks();
	int frames = 0;
	int cnt = 0;
	while(running){
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT || event.type == SDL_KEYUP){
				running = 0;
			}
		}
		int x, y;
		if(SDL_MUSTLOCK(screen)){
			SDL_LockSurface(screen);
		}
		for(x = 0; x < SCRW; ++x){
			for(y = 0; y < SCRH; ++y){
				putpixel(screen,x,y,pal[128+(sine[(2*y-(cnt>>2)-64)&0xFF]+sine[(2*x-(cnt>>2))&0xFF]+sine[(y +(cnt>>2))&0xFF]+sine[(x+(cnt>>2)-64)&0xFF])/4]);
			}
		}
		if(SDL_MUSTLOCK(screen)){
			SDL_UnlockSurface(screen);
		}
		cnt+=2;
		SDL_Flip(screen);
		++frames;
		if(!(frames&0xFF)){
			printf("%i FPS\n",(1000*frames)/(SDL_GetTicks()-start));
		}
	}
	SDL_Quit();
	return 0;
}
