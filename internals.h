#ifndef INTERNALS_H
#define INTERNALS_H

#define SVG_VERSION 120

#include <libsvg/svg.h>

#ifdef VERBOSE
#define dprintf(format, arg...) printf(format, ##arg)
#else
#define dprintf(format, arg...)
#endif

// Point. This is in coordinates appropriate to the SDL_Surface
typedef struct _IPoint {
	float x,y;
} IPoint;

typedef struct svg_matrix {
	float a,b,c,d,e,f;
} svg_matrix_t;


#define MATRIXSTACKDEPTH 16 // must be power of 2

#define MINPATH 256 // when we allocate a path, start with this number of points
#define MINPATHSTOPS 64

#define INT_FLAG_CLIPPING_SET      1

#define NUM_GRADIENT_COLORS 256 // must be power of 2
struct SDL_svg_context {
	SDL_Surface *surface;
	unsigned long flags;
	unsigned long internal_flags;
	int numpoints; // number of ipoints of path that is building up
	int pathmax; // number of ipoints that will fit in the allocated path
	IPoint *path; // path that is building up
	char *tags; // tags used for gray renderer, same # as path
	int numpathstops; // number of pathstops at *pathstops
	int maxpathstops; // maximum space at *pathstops
	short *pathstops; // the pathstops, each are which IPoint at path to stop
	IPoint at;
	double FillOpacity;
// svg_paint_t
	void *paint; // Big question who deallocates this
	void *paintstack[MATRIXSTACKDEPTH];
	int paintsp;
	IPoint gradient_p1; // for radial, the center point
	IPoint gradient_p2; // for radial, the focus point
	float gradient_r;
	int gradient_policy;
	int tmatrixsp;
	svg_matrix_t tmatrixstack[MATRIXSTACKDEPTH];
	Uint32 gradient_colors[NUM_GRADIENT_COLORS];
	Uint32 solidcolor;
// stuff related to the gray raster engine from freetype
	void (*renderfunc)(SDL_svg_context *c, void *span, int y);
	void (*colordot)(SDL_Surface *, int x, int y, Uint32 c, int f2);
	char pool[0x40000]; // memory area used by gray raster engine
//
	svg_matrix_t gm;
	float minx, miny, maxx, maxy;

// svg_fill_rule_t
	int fill_rule;

	float w,h;
	double OffsetX,OffsetY;
	double ScaleX,ScaleY;
//svg_t *
	void *SVG;
	int TargetOffsetX;
	int TargetOffsetY;

	int clip_xmin, clip_ymin, clip_xmax, clip_ymax; // in pixel coords

};

IPoint FixCoords(SDL_svg_context *c, IPoint p);
IPoint FixSizes(SDL_svg_context *c, IPoint p);
float ConvertLength(svg_length_t *l);

void svg_matrix_init(svg_matrix_t *dst, float va, float vb, float vc,
			float vd, float ve, float vf);
IPoint svg_apply_matrix(svg_matrix_t *m, IPoint p);
void svg_matrix_translate(svg_matrix_t *m, float dx, float dy);
void svg_matrix_scale(svg_matrix_t *m, float sx, float sy);
IPoint svg_apply_matrix_without_translation(svg_matrix_t *m, IPoint p);

void svg_matrix_multiply(svg_matrix_t *dest, svg_matrix_t *left, svg_matrix_t *right);
void svg_render_solid(SDL_svg_context *c);
svg_matrix_t svg_matrix_invert(svg_matrix_t *in);

#endif
