#ifndef SDL_SVG_H
#define SDL_SVG_H

#include "SDL.h"

typedef struct SDL_svg_context SDL_svg_context;

#define SDL_SVG_FLAG_DIRECT         0
#define SDL_SVG_FLAG_COMPOSITE      1

SDL_svg_context *SVG_Load(const char *FileName);
SDL_svg_context *SVG_LoadBuffer(char *data, int len);
int SVG_SetOffset(SDL_svg_context *Source, double aOffsetX, double aOffsetY);
int SVG_SetScale(SDL_svg_context *Source, double aScaleX,  double aScaleY);
int SVG_RenderToSurface(SDL_svg_context *Source, int X, int Y, SDL_Surface* Target);
void SVG_Free(SDL_svg_context *Source);
void SVG_Set_Flags(SDL_svg_context *c, unsigned long flags);

SDL_svg_context *create_SDL_svg_context(void);
void destroy_SDL_svg_context(SDL_svg_context *c);

float SVG_Width(SDL_svg_context *c);
float SVG_Height(SDL_svg_context *c);
int SVG_Version(void);

#endif
