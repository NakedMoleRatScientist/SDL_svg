#include "SDL_svg.h"
#include "SDL.h"
#include <signal.h>
#include <stdlib.h>

#define WIDTH 640
#define HEIGHT 480

int main(int argc, char *argv[])
{
int dx=0, dy=0;
SDL_Surface *screen;
int done;
SDL_Event event;
SDL_Rect full;
double Scalex, Scaley;
Uint32 Time;
int redraw=0;
int code;

	if(argc<2)
	{
		printf("Usage: %s <filename> [dx] [dy]\n",argv[0]);
		exit(0);
	}

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	/* Set 640x480 16 bpp video mode */
	if ( (screen=SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_SWSURFACE)) == NULL )
	{
		fprintf(stderr, "Couldn't set %dx%dx32 video mode: %s\n", WIDTH, HEIGHT,
              SDL_GetError());
		exit(2);
	}
	signal(SIGINT, SIG_DFL);
	full.x = 0;
	full.y = 0;
	full.w = screen->w;
	full.h = screen->h;
	SDL_FillRect(screen, &full, 0x808080);
	SDL_UpdateRect(screen,0,0,0,0);

	SDL_svg_context *TestImage = SVG_Load (argv[1]);
	SVG_SetOffset (TestImage,SVG_Width(TestImage)/2,SVG_Height(TestImage)/2);

	if(argc>2) sscanf(argv[2], "%d", &dx);
	if(argc>3) sscanf(argv[3], "%d", &dy);

	Scalex = screen->w / SVG_Width(TestImage);
	Scaley = screen->h / SVG_Height(TestImage);
	SVG_SetScale (TestImage, Scalex, Scaley);
	SVG_RenderToSurface (TestImage,screen->w/2+dx,screen->h/2+dy,screen);

	SDL_UpdateRect(screen,0,0,0,0);
	Time = SDL_GetTicks();
	/* Wait for a keystroke */
	done = 0;
	while ( !done )
	{
		SDL_Delay(10);
		/* Check for events */
		while ( SDL_PollEvent(&event) )
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				code=event.key.keysym.sym;
#define FACTOR 1.1
				if(code=='+' || code=='=')
				{
					Scalex*=FACTOR;
					Scaley*=FACTOR;
					redraw=1;
				}
				if(code=='-')
				{
					Scalex/=FACTOR;
					Scaley/=FACTOR;
					redraw=1;
				}
				if(code==0x1b) // escape
					done = 1;
				break;
			case SDL_QUIT:
				done = 1;
				break;
			default:
				break;
			}
		}
		if(redraw)
		{
			full.x = 0;
			full.y = 0;
			full.w = screen->w;
			full.h = screen->h;
			SDL_FillRect(screen, &full, 0xFFFFFF);
			SVG_SetScale (TestImage,Scalex,Scaley);
			SVG_RenderToSurface (TestImage,screen->w/2,screen->h/2,screen);
			SDL_UpdateRect(screen,0,0,0,0);
			redraw=0;
		}

	}
	SVG_Free (TestImage);
	return 0;
}
