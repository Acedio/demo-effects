#include <SDL/SDL.h>
#include <math.h>
#include <stdio.h>

#define SCRW 320
#define SCRH 240
#define SCRBPP 24
#define SHIFT 1

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

/*void putpixel(SDL_Surface* dst, int x, int y, Uint32 pxl){
	((Uint8*)dst->pixels)[x*dst->format->BytesPerPixel+y*dst->pitch] = pxl;
}

Uint32 getpixel(SDL_Surface* src, int x, int y){
	return ((Uint8*)src->pixels)[x*src->format->BytesPerPixel+y*src->pitch];
}*/

void largen(SDL_Surface* dst, SDL_Surface* src, int shift){
	int x, y;
	int w = src->clip_rect.w,h = src->clip_rect.h;
	SDL_Rect pixel;
	pixel.w = shift + 1;
	pixel.h = shift + 1;
	for(x = 0; x < w; ++x){
		pixel.x = x << shift;
		for(y = 0; y < h; ++y){
			pixel.y = y << shift;
			SDL_FillRect(dst,&pixel,getpixel(src,x,y));
		}
	}
}

void sinify(SDL_Surface* dst, SDL_Rect* dst_rect, SDL_Surface* src, float amp, float period, float t){
	int i, j;

	if(SDL_MUSTLOCK(src)){
		SDL_LockSurface(src);
	}
	if(SDL_MUSTLOCK(dst)){
		SDL_LockSurface(dst);
	}

	for(i = 0; i < src->clip_rect.w; i++){
		int amt = (int)(amp*(1+sin(t+(float)i/period)));
		for(j = 0; j < src->clip_rect.h; j++){
			putpixel(dst,i+dst_rect->x,j+dst_rect->y+amt,getpixel(src,i,j));
		}
	}

	if(SDL_MUSTLOCK(dst)){
		SDL_UnlockSurface(dst);
	}
	if(SDL_MUSTLOCK(src)){
		SDL_UnlockSurface(src);
	}
}

void plasmate(SDL_Surface* dst, float xr, float yr, float t){
	int i, j;
	if(SDL_MUSTLOCK(dst)){
		SDL_LockSurface(dst);
	}

	for(i = 0; i < dst->clip_rect.w; i++){
		float ival = 63*cos(t+xr*i);
		for(j = 0; j < dst->clip_rect.h; j++){
			int val = 128 + (int)(ival + 63*sin(t+yr*j));
			putpixel(dst,i,j,SDL_MapRGB(dst->format,val/(2.1+sin(t)*2),val,val/(2.1+cos(t)*2)));
		}
	}

	if(SDL_MUSTLOCK(dst)){
		SDL_UnlockSurface(dst);
	}
}
			

void rotozoom(SDL_Surface* dst, SDL_Surface* src, float angle, float scale){
	float src_x, src_y;
	int dst_x, dst_y;
	float dx, dy;
	float start_x = 0, start_y = 0;
	int x_mask = src->clip_rect.w - 1;
	int y_mask = src->clip_rect.h - 1;

	dx = cos(angle) * scale;
	dy = sin(angle) * scale;

	if(SDL_MUSTLOCK(src)){
		SDL_LockSurface(src);
	}
	if(SDL_MUSTLOCK(dst)){
		SDL_LockSurface(dst);
	}

	for(dst_y = 0; dst_y < dst->clip_rect.h; dst_y++){
		src_x = start_x;
		src_y = start_y;

		for(dst_x = 0; dst_x < dst->clip_rect.w; dst_x++){
			putpixel(dst,dst_x,dst_y,getpixel(src,(int)(src_x)&x_mask,(int)(src_y)&y_mask));
			src_x += dx;
			src_y += dy;
		}
		start_x -= dy;
		start_y += dx;
	}

	if(SDL_MUSTLOCK(dst)){
		SDL_UnlockSurface(dst);
	}
	if(SDL_MUSTLOCK(src)){
		SDL_UnlockSurface(src);
	}
}

int main(){
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Surface* screen = SDL_SetVideoMode(SCRW,SCRH,SCRBPP,SDL_SWSURFACE);
	SDL_Rect* dest;
	dest->w = 16;
	dest->h = 16;
	SDL_Surface* tile = SDL_DisplayFormat(SDL_LoadBMP("tile.bmp"));
	SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 128, 128, SCRBPP,0,0,0,0);
	SDL_Event event;
	int running = 1;
	float theta = 0;
	SDL_Rect cent;
	cent.x = 96;
	cent.y = 24;
	Uint32 start = SDL_GetTicks();
	int frames = 0;

	while(running){
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
				case SDL_KEYUP:
					running = 0;
					break;
				default:
					break;
			}
		}
		int x,y;
		rotozoom(screen,tile,theta,1+sin(theta));
		//plasmate(screen,.05*cos(theta*.8),.09*sin(theta),10*theta);
		rotozoom(temp,tile,0,.125);
		sinify(screen,&cent,temp,32,64,theta*4);
		theta += 3.14159/1000;
		SDL_Flip(screen);
		frames += 1;
		if(frames%100 == 0){
			printf("%i\n",(1000*frames)/(SDL_GetTicks()-start));
		}
	}
	SDL_FreeSurface(tile);
	SDL_Quit();
	return 0;
}
