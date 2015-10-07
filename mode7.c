#include <stdio.h>
#include <math.h>
#include <SDL/SDL.h>
#include "pixel.h"

#define SCRW 320
#define SCRH 240
#define SCRBPP 24

struct mode7params {
	float d_to_scr, scale_x, scale_y;
	int tilemask;
};

void mode7(SDL_Surface* dst, SDL_Surface* src, float height, float angle, float x_pos, float y_pos, struct mode7params m7p){
	int x,y;
	float d, dx, dy = 0;
	for(y = 0; y < dst->h; ++y){
		float current = (height * (m7p.d_to_scr/((float)y)));
		if(0 && current > 1800){
			continue;
		}
		float start = (current*(-.5*(float)dst->w)/m7p.d_to_scr);
		float current_x = x_pos + cos(angle)*current + sin(angle)*start;
		float current_y = y_pos + sin(angle)*current - cos(angle)*start;
		d = current/m7p.d_to_scr;
		dx = sin(angle)*d;
		dy = -cos(angle)*d;
		for(x = 0; x < dst->w; ++x){
			putpixel(dst,x,y,getpixel(src,((int)(current_x/m7p.scale_x))&(m7p.tilemask),((int)(current_y/m7p.scale_y))&(m7p.tilemask)));
			current_x += dx;
			current_y += dy;
		}
	}
}

int main(){
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Surface* screen = SDL_SetVideoMode(SCRW,SCRH,SCRBPP,SDL_SWSURFACE);
	SDL_Surface* tile = SDL_DisplayFormat(SDL_LoadBMP("tile.bmp"));

	SDL_Event event;

	int running = 1;

	float height = 320;
	float angle = 3.14159/2;
	float vel = 4;
	float x_pos = 0, y_pos = 0;

	struct mode7params m7p;
	m7p.d_to_scr = 235;
	m7p.scale_x = 4;
	m7p.scale_y = 4;
	m7p.tilemask = tile->w-1;

	Uint32 start = SDL_GetTicks();
	Uint32 fpstime = start;
	int frames = 0;
	int FPS = 60;
	int MSPF = 1000/FPS;

	while(running){
		Uint8* keystates = SDL_GetKeyState(NULL);
		if(keystates[SDLK_UP]){
			m7p.d_to_scr -= 5;
			printf("%f\n",m7p.d_to_scr);
		}
		if(keystates[SDLK_DOWN]){
			m7p.d_to_scr += 5;
			printf("%f\n",m7p.d_to_scr);
		}
		if(keystates[SDLK_LEFT]){
			angle += .01;
		}
		if(keystates[SDLK_RIGHT]){
			angle -= .01;
		}
		if(keystates[SDLK_PAGEUP]){
			height += 5;
		}
		if(keystates[SDLK_PAGEDOWN]){
			height -= 5;
		}
		if(keystates[SDLK_w]){
			vel += .1;
		}
		if(keystates[SDLK_s]){
			vel -= .1;
		}
		if(keystates[SDLK_a]){
			angle += .01;
		}
		if(keystates[SDLK_d]){
			angle -= .01;
		}
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				running = 0;
			}
		}
		x_pos += cos(angle)*vel;
		y_pos += sin(angle)*vel;
		mode7(screen, tile, height, angle, x_pos, y_pos, m7p);
		SDL_Flip(screen);
		frames++;
		if(frames%100 == 0){
			printf("%i\n",frames*1000/(SDL_GetTicks()-fpstime));
		}
		Uint32 elapsed = SDL_GetTicks() - start;
		if(elapsed < MSPF){
			SDL_Delay(MSPF - elapsed);
		}
		start = SDL_GetTicks();
	}

	SDL_FreeSurface(screen);
	SDL_FreeSurface(tile);
	SDL_Quit();
	return 0;
}
