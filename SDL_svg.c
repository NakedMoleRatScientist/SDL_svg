#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "SDL_svg.h"
#include "internals.h"
#include <svg.h>

static svg_status_t _SDL_SVG_BeginGroup (void *closure, double opacity);
static svg_status_t _SDL_SVG_BeginElement (void *closure);
static svg_status_t _SDL_SVG_EndElement (void *closure);
static svg_status_t _SDL_SVG_EndGroup (void *closure, double opacity);
static svg_status_t _SDL_SVG_MoveTo (void *closure, double x, double y);
static svg_status_t _SDL_SVG_LineTo (void *closure, double x, double y);
static svg_status_t _SDL_SVG_CurveTo (void *closure,
                                      double x1, double y1,
                                      double x2, double y2,
                                      double x3, double y3);
static svg_status_t _SDL_SVG_QuadraticCurveTo (void *closure,
                                               double x1, double y1,
                                               double x2, double y2);
static svg_status_t _SDL_SVG_ArcTo (void *closure, double rx, double ry,
                                    double	x_axis_rotation,
                                    int large_arc_flag,
                                    int sweep_flag,
                                    double	x, double y);
static svg_status_t _SDL_SVG_ClosePath (void *closure);
static svg_status_t _SDL_SVG_SetColor (void *closure,
                                       const svg_color_t *color);
static svg_status_t _SDL_SVG_SetFillOpacity (void *closure,
                                             double fill_opacity);
static svg_status_t _SDL_SVG_SetFillPaint (void *closure,
                                           const svg_paint_t *paint);
static svg_status_t _SDL_SVG_SetFillRule (void *closure,
                                          svg_fill_rule_t fill_rule);
static svg_status_t _SDL_SVG_SetFontFamily (void *closure,
                                            const char *family);
static svg_status_t _SDL_SVG_SetFontSize (void *closure, double size);
static svg_status_t _SDL_SVG_SetFontStyle (void *closure,
                                           svg_font_style_t font_style);
static svg_status_t _SDL_SVG_SetFontWeight (void *closure,
                                            unsigned int weight);
static svg_status_t _SDL_SVG_SetOpacity (void *closure, double opacity);
static svg_status_t _SDL_SVG_SetStrokeDashArray (void *closure,
                                                 double *dash,
                                                 int num_dashes);
static svg_status_t _SDL_SVG_SetStrokeDashOffset (void *closure,
                                                  svg_length_t *offset);
static svg_status_t _SDL_SVG_SetStrokeLineCap (void *closure,
                                               svg_stroke_line_cap_t line_cap);
static svg_status_t _SDL_SVG_SetStrokeLineJoin (void *closure,
                                                svg_stroke_line_join_t line_join);
static svg_status_t _SDL_SVG_SetStrokeMiterLimit (void *closure,
                                                  double limit);
static svg_status_t _SDL_SVG_SetStrokeOpacity (void *closure,
                                               double stroke_opacity);
static svg_status_t _SDL_SVG_SetStrokePaint (void *closure,
                                             const svg_paint_t *paint);
static svg_status_t _SDL_SVG_SetStrokeWidth (void *closure,
                                             svg_length_t *width);
static svg_status_t _SDL_SVG_SetTextAnchor (void *closure,
                                            svg_text_anchor_t text_anchor);
static svg_status_t _SDL_SVG_Transform (void *closure,
                                        double a, double b, double c,
                                        double d, double e, double f);
static svg_status_t _SDL_SVG_ApplyViewBox (void *closure,
                                           svg_view_box_t view_box,
                                           svg_length_t *width,
                                           svg_length_t *height);
static svg_status_t _SDL_SVG_SetViewportDimension (void *closure,
                                                   svg_length_t *width,
                                                   svg_length_t *height);
static svg_status_t _SDL_SVG_RenderLine (void *closure,
                                         svg_length_t *x1_len,
                                         svg_length_t *y1_len,
                                         svg_length_t *x2_len,
                                         svg_length_t *y2_len);
static svg_status_t _SDL_SVG_RenderPath (void *closure);
static svg_status_t _SDL_SVG_RenderEllipse (void *closure,
                                            svg_length_t *cx,
                                            svg_length_t *cy,
                                            svg_length_t *rx,
                                            svg_length_t *ry);
static svg_status_t _SDL_SVG_RenderRect (void *closure,
                                         svg_length_t *x,
                                         svg_length_t *y,
                                         svg_length_t *width,
                                         svg_length_t *height,
                                         svg_length_t *rx,
                                         svg_length_t *ry);
static svg_status_t _SDL_SVG_RenderText (void *closure,
                                         svg_length_t *x_len,
                                         svg_length_t *y_len,
                                         unsigned char *utf8);
static svg_status_t _SDL_SVG_RenderImage (void *closure,
                                          unsigned char *data,
                                          unsigned int data_width,
                                          unsigned int data_height,
                                          svg_length_t *x,
                                          svg_length_t *y,
                                          svg_length_t *width,
                                          svg_length_t *height);
static svg_render_engine_t SDL_SVG_RenderEngine = {
  /* hierarchy */
  _SDL_SVG_BeginGroup,
  _SDL_SVG_BeginElement,
  _SDL_SVG_EndElement,
  _SDL_SVG_EndGroup,
  /* path creation */
  _SDL_SVG_MoveTo,
  _SDL_SVG_LineTo,
  _SDL_SVG_CurveTo,
  _SDL_SVG_QuadraticCurveTo,
  _SDL_SVG_ArcTo,
  _SDL_SVG_ClosePath,
  /* style */
  _SDL_SVG_SetColor,
  _SDL_SVG_SetFillOpacity,
  _SDL_SVG_SetFillPaint,
  _SDL_SVG_SetFillRule,
  _SDL_SVG_SetFontFamily,
  _SDL_SVG_SetFontSize,
  _SDL_SVG_SetFontStyle,
  _SDL_SVG_SetFontWeight,
  _SDL_SVG_SetOpacity,
  _SDL_SVG_SetStrokeDashArray,
  _SDL_SVG_SetStrokeDashOffset,
  _SDL_SVG_SetStrokeLineCap,
  _SDL_SVG_SetStrokeLineJoin,
  _SDL_SVG_SetStrokeMiterLimit,
  _SDL_SVG_SetStrokeOpacity,
  _SDL_SVG_SetStrokePaint,
  _SDL_SVG_SetStrokeWidth,
  _SDL_SVG_SetTextAnchor,
  /* transform */
  _SDL_SVG_Transform,
  _SDL_SVG_ApplyViewBox,
  _SDL_SVG_SetViewportDimension,
  /* drawing */
  _SDL_SVG_RenderLine,
  _SDL_SVG_RenderPath,
  _SDL_SVG_RenderEllipse,
  _SDL_SVG_RenderRect,
  _SDL_SVG_RenderText,
  _SDL_SVG_RenderImage
};

void pushtmatrix(SDL_svg_context *c)
{
int nsp;
	nsp = (c->tmatrixsp + 1) & (MATRIXSTACKDEPTH-1);
	c->tmatrixstack[nsp]=c->tmatrixstack[c->tmatrixsp];
	c->tmatrixsp = nsp;
}
void poptmatrix(SDL_svg_context *c)
{
	c->tmatrixsp = (c->tmatrixsp - 1) & (MATRIXSTACKDEPTH-1);
}


IPoint FixCoords(SDL_svg_context *c, IPoint p)
{
	p=svg_apply_matrix(c->tmatrixstack + c->tmatrixsp, p);

if(0)
{
static float xmin=0x7fffffff, xmax=-0x7fffffff,
	 ymin=0x7fffffff, ymax=-0x7ffffff;
int changed=0;

	if(p.x < xmin) xmin=p.x, changed=1;
	if(p.y < ymin) ymin=p.y, changed=1;
	if(p.x > xmax) xmax=p.x, changed=1;
	if(p.y > ymax) ymax=p.y, changed=1;
	if(1 || changed)
	{
		printf("(%f,%f) - (%f,%f)\n", xmin, ymin, xmax, ymax);
	}
}

	return (IPoint) {
		(p.x - c->OffsetX) * c->ScaleX + c->TargetOffsetX,
		(p.y - c->OffsetY) * c->ScaleY + c->TargetOffsetY};
}

IPoint FixSizes(SDL_svg_context *c, IPoint p)
{
	p=svg_apply_matrix_without_translation(c->tmatrixstack + c->tmatrixsp, p);
	return (IPoint) { p.x * c->ScaleX, p.y * c->ScaleY};
}

static void _AddIPoint(SDL_svg_context *c, IPoint p)
{
	if(c->numpoints >= c->pathmax)
	{
		c->pathmax<<=1;
		if(c->pathmax < MINPATH)
			c->pathmax=MINPATH;
		c->path = realloc(c->path, c->pathmax * sizeof(IPoint));
// WARNING, possible memory allocation failure, untested
	}
	c->path[c->numpoints++] = p;
}

// Note the explicitclose I'd plan on adding a high order bit set to
// 1 on the entry into the pathstops table, if explicitclose is
// set to true. Since we're not rendering the path it doesn't matter,
// but if we were rendering the path we'd need to know how the final
// join is handled, and that has to do whether there is a closepath.
// (DA) 20050115
static void _AddPathStop(SDL_svg_context *c, int explicitclose)
{
	if(c->numpathstops >= c->maxpathstops)
	{
		c->maxpathstops<<=1;
		if(c->maxpathstops<MINPATHSTOPS)
			c->maxpathstops=MINPATHSTOPS;
		c->pathstops = realloc(c->pathstops,
					c->maxpathstops * sizeof(int));
// WARNING, possible memory allocation failure, untested
	}
	c->pathstops[c->numpathstops++] = c->numpoints;
}

static int needs_path_stop(SDL_svg_context *c)
{
	return !c->numpathstops ||
			c->pathstops[c->numpathstops-1] < c->numpoints;
}

static svg_status_t
_SDL_SVG_SetViewportDimension (void *closure,
							   svg_length_t *width,
							   svg_length_t *height)
{
	dprintf("svg_SetViewportDimension (width=%5.5f, height=%5.5f)\n",
          width->value,height->value);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t _SDL_SVG_BeginGroup (void *closure, double opacity)
{
	dprintf("svg_BeginGroup (Opacity=%5.5f)\n",opacity);
	pushtmatrix((SDL_svg_context *)closure);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t _SDL_SVG_BeginElement (void *closure)
{
	dprintf("svg_BeginElement\n");
	pushtmatrix((SDL_svg_context *)closure);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_EndElement (void *closure)
{
	dprintf("svg_EndElement\n");
	poptmatrix((SDL_svg_context *)closure);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_EndGroup (void *closure, double opacity)
{
	dprintf("svg_EndGroup (opacity=%5.5f)\n",opacity);
	poptmatrix((SDL_svg_context *)closure);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_MoveTo (void *closure, double x, double y)
{
SDL_svg_context *c=closure;
	dprintf("svg_MoveTo (x=%5.5f, y=%5.5f)\n",x,y);

	if(c->numpoints && needs_path_stop(c))
		_AddPathStop(c, 0);
	_AddIPoint(c, FixCoords(c, (IPoint) {x, y}));
	c->at = (IPoint) {x, y};

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_LineTo (void *closure, double x, double y)
{
SDL_svg_context *c=closure;

	dprintf("svg_LineTo (x=%5.5f, y=%5.5f)\n",x,y);

	_AddIPoint(c, FixCoords(c, (IPoint) {x, y}));
	c->at = (IPoint) {x, y};

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_CurveTo (void *closure,
				  double x1, double y1,
				  double x2, double y2,
				  double x3, double y3)
{
SDL_svg_context *c=closure;
double t, k = .0625;								// size of drawing step
IPoint p0,p1,p2,p3;

	dprintf("svg_CurveTo (x1=%5.5f, y1=%5.5f, x2=%5.5f, y2=%5.5f, x3=%5.5f, y3=%5.5f)\n",
          x1,y1,x2,y2,x3,y3);

	if(!c->path || !c->numpoints)
		return SVG_STATUS_INVALID_CALL;

	p0 = c->path[c->numpoints-1];

	p1 = FixCoords(c, (IPoint) {x1, y1});
	p2 = FixCoords(c, (IPoint) {x2, y2});
	p3 = FixCoords(c, (IPoint) {x3, y3});

	for(t = k; t <= 1+k; t += k)					// calculate and draw
	{											// Bezier curve using
		float t2 = t * t;							// Berstein polynomials
		float t3 = t * t2;
		float t4 = (1 - 3*t + 3*t2 - t3);
		float t5 = (3*t - 6*t2 + 3*t3);
		float t6 = (3*t2 - 3*t3);
		float nx = t4*p0.x + t5*p1.x + t6*p2.x + t3*p3.x;
		float ny = t4*p0.y + t5*p1.y + t6*p2.y + t3*p3.y;

		_AddIPoint(c, (IPoint) {nx, ny});
	}
	c->at = (IPoint) {x3, y3};
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_QuadraticCurveTo (void *closure,
						  double x1, double y1,
						  double x2, double y2)
{
	dprintf("svg_QuadraticCurveTo (x1=%5.5f, y1=%5.5f, x2=%5.5f, y2=%5.5f)\n",
          x1,y1,x2,y2);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_ClosePath (void *closure)
{
SDL_svg_context *c=closure;

	dprintf("svg_ClosePath\n");

	_AddPathStop(c, 1);

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetColor (void *closure, const svg_color_t *color)
{
	dprintf("svg_SetColor (color=#%08x)\n",color->rgb);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFillOpacity (void *closure, double fill_opacity)
{
SDL_svg_context *c=closure;
	dprintf("svg_SetFillOpacity (fill_opacity=%5.5f)\n",fill_opacity);

	c->FillOpacity = fill_opacity;

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFillPaint (void *closure, const svg_paint_t *paint)
{
SDL_svg_context *c=closure;

	dprintf("svg_SetFillPaint\n");

	c->paint = (void *)paint;

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFillRule (void *closure, svg_fill_rule_t fill_rule)
{
SDL_svg_context *c=closure;
	dprintf("svg_SetFillRule %d\n", fill_rule);
	c->fill_rule = fill_rule;
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFontFamily (void *closure, const char *family)
{
	dprintf("svg_SetFontFamily\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFontSize (void *closure, double size)
{
	dprintf("svg_SetFontSize\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFontStyle (void *closure, svg_font_style_t font_style)
{
	dprintf("svg_SetFontStyle\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetFontWeight (void *closure, unsigned int font_weight)
{
	dprintf("svg_SetFontWeight\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetOpacity (void *closure, double opacity)
{
	dprintf("svg_SetOpacity  %f\n",opacity);
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeDashArray (void *closure, double *dash, int num_dashes)
{
	dprintf("svg_SetStrokeDashArray\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeDashOffset (void *closure, svg_length_t *offset_len)
{
	dprintf("svg_SetStrokeDashOffset\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeLineCap (void *closure, svg_stroke_line_cap_t line_cap)
{
	dprintf("svg_SetStrokeLineCap\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeLineJoin (void *closure, svg_stroke_line_join_t line_join)
{
	dprintf("svg_SetStrokeLineJoin\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeMiterLimit (void *closure, double limit)
{
	dprintf("svg_SetStrokeMiterLimit\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeOpacity (void *closure, double stroke_opacity)
{
	dprintf("svg_SetStrokeOpacity\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokePaint (void *closure, const svg_paint_t *paint)
{
//svg_color_t *color = (svg_color_t*)(&(paint->p.color));
	dprintf("svg_SetStrokePaint\n");

	if (paint->type == SVG_PAINT_TYPE_GRADIENT)
	{
		if (paint->p.gradient->type == SVG_GRADIENT_LINEAR)
			dprintf ("svg_SetStrokePaint  SVG_GRADIENT_LINEAR (color=#%08x) opacity %f\n", paint->p.gradient->stops->color.rgb, paint->p.gradient->stops->opacity);
		else
			dprintf ("svg_SetStrokePaint  SVG_GRADIENT_RADIAL(color=#%08x) opacity %f\n",paint->p.gradient->stops->color.rgb, paint->p.gradient->stops->opacity);
	} else
		dprintf("svg_SetStrokePaint (color=#%08x)\n",paint->p.color.rgb);

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetStrokeWidth (void *closure, svg_length_t *width_len)
{
	dprintf("svg_SetStrokeWidth\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_SetTextAnchor (void *closure, svg_text_anchor_t text_anchor)
{
	dprintf("svg_SetTextAnchor\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_Transform (void *closure,
					double va, double vb,
					double vc, double vd,
					double ve, double vf)
{
SDL_svg_context *c=closure;
svg_matrix_t *m, temp;
	dprintf("svg_Transform (%f %f %f %f  %f %f)\n",va,vb,vc,vd,ve,vf);

	m=c->tmatrixstack + c->tmatrixsp;
	svg_matrix_init(&temp, va, vb, vc, vd, ve, vf);
	svg_matrix_multiply(m, m, &temp);

//	svg_matrix_init (c->tmatrixstack + c->tmatrixsp, va, vb, vc, vd, ve, vf);

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_RenderLine (void *closure,
					svg_length_t *x1_len,
					svg_length_t *y1_len,
					svg_length_t *x2_len,
					svg_length_t *y2_len)
{
	dprintf("svg_RenderLine\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_RenderPath (void *closure)
{
SDL_svg_context *c=closure;
	dprintf("svg_RenderPath\n");

	if(needs_path_stop(c))
		_AddPathStop(c, 0);
	solid(c);

	c->numpoints=0;
	c->numpathstops=0;

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_RenderEllipse (void *closure,
						svg_length_t *cx_len,
						svg_length_t *cy_len,
						svg_length_t *rx_len,
						svg_length_t *ry_len)
{
SDL_svg_context *c=closure;
IPoint center;
IPoint size1, size2;
	dprintf("svg_RenderEllipse\n");

	center = FixCoords(c, (IPoint)
			{c->at.x + cx_len->value, c->at.y + cy_len->value});

	size1 = FixSizes(c, (IPoint) { rx_len->value, 0});
	size2 = FixSizes(c, (IPoint) { 0, ry_len->value});

//	filledEllipseRGBA(c->surface, center.x, center.y, size.x, size.y, R,G,B,A);
//	aaellipseRGBA(c->surface, center.x, center.y, size.x, size.y, R,G,B,A);

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_RenderRect (void *closure,
					svg_length_t *x_len,
					svg_length_t *y_len,
					svg_length_t *width_len,
					svg_length_t *height_len,
					svg_length_t *rx_len,
					svg_length_t *ry_len)
{
SDL_svg_context *c=closure;
float x1,y1;
float x2,y2;

	dprintf("svg_RenderRect\n");

	x1 = x_len->value;
	y1 = y_len->value;

	x2 = x1 + width_len->value;
	y2 = y1 + height_len->value;

	_AddIPoint(c, FixCoords(c, (IPoint) {x1, y1}));
	_AddIPoint(c, FixCoords(c, (IPoint) {x2, y1}));
	_AddIPoint(c, FixCoords(c, (IPoint) {x2, y2}));
	_AddIPoint(c, FixCoords(c, (IPoint) {x1, y2}));
	_SDL_SVG_RenderPath(closure);

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_RenderText (void *closure,
					svg_length_t *x_len,
					svg_length_t *y_len,
					unsigned char *utf8)
{
	dprintf("svg_RenderText\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t _SDL_SVG_RenderImage (void *closure,
                                          unsigned char	*data,
                                          unsigned int	data_width,
                                          unsigned int	data_height,
                                          svg_length_t	*x_len,
                                          svg_length_t	*y_len,
                                          svg_length_t	*width_len,
                                          svg_length_t	*height_len)
{
	dprintf("svg_RenderImage\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t _SDL_SVG_ApplyViewBox (void *closure,
                                           svg_view_box_t view_box,
                                           svg_length_t *width,
                                           svg_length_t *height)
{
	dprintf("svg_ApplyViewBox\n");
	return SVG_STATUS_SUCCESS;
}

static svg_status_t _SDL_SVG_ArcTo (void *closure,
                                    double rx,
                                    double ry,
                                    double x_axis_rotation,
                                    int    large_arc_flag,
                                    int    sweep_flag,
                                    double x,
                                    double y)
{
	dprintf("svg_ArcTo\n");
	return SVG_STATUS_SUCCESS;
}

float ConvertLength(svg_length_t *l)
{
float f=1.0;
	switch(l->unit)
	{
	case SVG_LENGTH_UNIT_CM:f=35.43307;break;
	case SVG_LENGTH_UNIT_EM:break;
	case SVG_LENGTH_UNIT_EX:break;
	case SVG_LENGTH_UNIT_IN:f=90.0;break;
	case SVG_LENGTH_UNIT_MM:f=3.543307;break;
	case SVG_LENGTH_UNIT_PC:f=15.0;break;
	case SVG_LENGTH_UNIT_PCT:break;
	case SVG_LENGTH_UNIT_PT:f=1.25;break;
	case SVG_LENGTH_UNIT_PX:f=1.0;break;
	}
	return l->value*f;
}

#define LOAD_TYPE_FILE     1
#define LOAD_TYPE_MEMORY   2

static SDL_svg_context *internal_SVG_Load(void *arg1, int arg2, int type)
{
svg_length_t Width;
svg_length_t Height;
SDL_svg_context *Source;

	Source = create_SDL_svg_context();
	svg_create ((svg_t **)&(Source->SVG));
	if(type==LOAD_TYPE_FILE)
		svg_parse (Source->SVG,(char *)arg1);
	else
		svg_parse_buffer(Source->SVG, (char *)arg1, arg2);
	svg_get_size (Source->SVG,&Width,&Height);
	Source->w = ConvertLength(&Width);
	Source->h = ConvertLength(&Height);

	return Source;

}

SDL_svg_context *SVG_Load(const char* FileName)
{
	return internal_SVG_Load((void *)FileName, 0, LOAD_TYPE_FILE);
}

SDL_svg_context *SVG_LoadBuffer(char *p, int len)
{
	return internal_SVG_Load(p, len, LOAD_TYPE_MEMORY);
}

int SVG_SetOffset(SDL_svg_context *Source, double aOffsetX, double aOffsetY)
{
	Source->OffsetX = aOffsetX;
	Source->OffsetY = aOffsetY;
	return SVG_STATUS_SUCCESS;
}

int SVG_SetScale(SDL_svg_context *Source, double aScaleX, double aScaleY)
{
	Source->ScaleX = aScaleX;
	Source->ScaleY = aScaleY;
	return SVG_STATUS_SUCCESS;
}

float SVG_Width(SDL_svg_context *c)
{
	return c->w;
}

float SVG_Height(SDL_svg_context *c)
{
	return c->h;
}

int SVG_RenderToSurface(SDL_svg_context *Source, int X, int Y,
		SDL_Surface *Target)
{
	Source->surface = Target;
	Source->TargetOffsetX = X;
	Source->TargetOffsetY = Y;

	return svg_render (Source->SVG, &SDL_SVG_RenderEngine, Source);
}

void SVG_Free(SDL_svg_context *Source)
{
	svg_destroy (Source->SVG);
	destroy_SDL_svg_context (Source);
}

SDL_svg_context *create_SDL_svg_context(void)
{
SDL_svg_context *c;
	c=calloc(1, sizeof(*c));
// WARNING, possible memory allocation failure, untested
	if(c)
	{
		c->OffsetX = 0.0;
		c->OffsetY = 0.0;
		c->ScaleX = 1.0;
		c->ScaleY = 1.0;
		c->tmatrixsp=0;
		svg_matrix_init(&c->tmatrixstack[c->tmatrixsp],
			1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
	}
	return c;
}

void destroy_SDL_svg_context(SDL_svg_context *c)
{
	if(c->path)
	{
		free(c->path);
		c->path=0;
	}
	free(c);
}
