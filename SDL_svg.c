#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "SDL_svg.h"
#include "internals.h"

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

void _scale(SDL_svg_context *c, float sx, float sy)
{
	svg_matrix_scale(c->tmatrixstack + c->tmatrixsp, sx, sy);
}

void _translate(SDL_svg_context *c, float dx, float dy)
{
	svg_matrix_translate(c->tmatrixstack + c->tmatrixsp, dx, dy);
}

static void _extremes(SDL_svg_context *c, float x, float y)
{
	if(x < c->minx) c->minx=x;
	if(y < c->miny) c->miny=y;
	if(x > c->maxx) c->maxx=x;
	if(y > c->maxy) c->maxy=y;
}

IPoint FixCoords(SDL_svg_context *c, IPoint p)
{
	p=svg_apply_matrix(c->tmatrixstack + c->tmatrixsp, p);

	return (IPoint) {
		(p.x - c->OffsetX) * c->ScaleX + c->TargetOffsetX,
		(p.y - c->OffsetY) * c->ScaleY + c->TargetOffsetY};
}

IPoint FixSizes(SDL_svg_context *c, IPoint p)
{
	p=svg_apply_matrix_without_translation(c->tmatrixstack + c->tmatrixsp, p);
	return (IPoint) { p.x * c->ScaleX, p.y * c->ScaleY};
}

#define TAG_ONPATH    1 // on the path
#define TAG_CONTROL2  0 // quadratic bezier control point
#define TAG_CONTROL3  2 // cubic bezier control point
static void _AddIPoint(SDL_svg_context *c, IPoint p, int tag)
{
	if(c->numpoints >= c->pathmax)
	{
		c->pathmax<<=1;
		if(c->pathmax < MINPATH)
			c->pathmax=MINPATH;
		c->path = realloc(c->path, c->pathmax * sizeof(IPoint));
		c->tags = realloc(c->tags, c->pathmax);
// WARNING, possible memory allocation failure, untested
	}
	c->tags[c->numpoints] = tag;
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
	c->pathstops[c->numpathstops++] = c->numpoints-1;
}

static int needs_path_stop(SDL_svg_context *c)
{
	return !c->numpathstops ||
			c->pathstops[c->numpathstops-1]+1 < c->numpoints;
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
SDL_svg_context *c=closure;
	dprintf("svg_BeginElement\n");
	pushtmatrix(c);
	c->minx = HUGE;
	c->miny = HUGE;
	c->maxx = -HUGE;
	c->maxy = -HUGE;
	if(c->paintsp < MATRIXSTACKDEPTH)
		c->paintstack[c->paintsp++] = c->paint;
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_EndElement (void *closure)
{
SDL_svg_context *c=closure;
	dprintf("svg_EndElement\n");
	poptmatrix(c);
	if(c->paintsp)
		c->paint = c->paintstack[--c->paintsp];
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
	_extremes(c, x, y);
	_AddIPoint(c, FixCoords(c, (IPoint) {x, y}), TAG_ONPATH);
	c->at = (IPoint) {x, y};

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_LineTo (void *closure, double x, double y)
{
SDL_svg_context *c=closure;

	dprintf("svg_LineTo (x=%5.5f, y=%5.5f)\n",x,y);

	_extremes(c, x, y);
	_AddIPoint(c, FixCoords(c, (IPoint) {x, y}), TAG_ONPATH);
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
IPoint p1,p2,p3;

	dprintf("svg_CurveTo (x1=%5.5f, y1=%5.5f, x2=%5.5f, y2=%5.5f, x3=%5.5f, y3=%5.5f)\n",
          x1,y1,x2,y2,x3,y3);

	if(!c->path || !c->numpoints)
		return SVG_STATUS_INVALID_CALL;

	_extremes(c, x1, y1);
	_extremes(c, x2, y2);
	_extremes(c, x3, y3);

	p1 = FixCoords(c, (IPoint) {x1, y1});
	p2 = FixCoords(c, (IPoint) {x2, y2});
	p3 = FixCoords(c, (IPoint) {x3, y3});

	_AddIPoint(c, (IPoint) {p1.x, p1.y}, TAG_CONTROL3);
	_AddIPoint(c, (IPoint) {p2.x, p2.y}, TAG_CONTROL3);
	_AddIPoint(c, (IPoint) {p3.x, p3.y}, TAG_ONPATH);

	c->at = (IPoint) {x3, y3};
	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_SDL_SVG_QuadraticCurveTo (void *closure,
						  double x1, double y1,
						  double x2, double y2)
{
SDL_svg_context *c=closure;
IPoint p1,p2;

	dprintf("svg_QuadraticCurveTo (x1=%5.5f, y1=%5.5f, x2=%5.5f, y2=%5.5f)\n",
          x1,y1,x2,y2);

	if(!c->path || !c->numpoints)
		return SVG_STATUS_INVALID_CALL;

	_extremes(c, x1, y1);
	_extremes(c, x2, y2);

	p1 = FixCoords(c, (IPoint) {x1, y1});
	p2 = FixCoords(c, (IPoint) {x2, y2});

	_AddIPoint(c, (IPoint) {p1.x, p1.y}, TAG_CONTROL2);
	_AddIPoint(c, (IPoint) {p2.x, p2.y}, TAG_ONPATH);

	c->at = (IPoint) {x2, y2};
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
	svg_render_solid(c);

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
svg_length_t rx2, ry2;
svg_length_t tcx, tcy;

	dprintf("svg_RenderEllipse\n");

	rx2 = *rx_len;
	rx2.value *= 2.0;

	ry2 = *ry_len;
	ry2.value *= 2.0;

	tcx = *cx_len;
	tcx.value -= rx_len->value;

	tcy = *cy_len;
	tcy.value -= ry_len->value;

	_SDL_SVG_RenderRect(c,
			&tcx, &tcy,
			&rx2, &ry2,
			rx_len, ry_len);

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
double x, y, width, height, rx, ry;

	dprintf("svg_RenderRect\n");

 
	x = ConvertLength(x_len);
	y = ConvertLength(y_len);
	width = ConvertLength(width_len);
	height = ConvertLength(height_len);
	rx = ConvertLength(rx_len);
	ry = ConvertLength(ry_len);
 
	if (rx > width / 2.0)
		rx = width / 2.0;
	if (ry > height / 2.0)
		ry = height / 2.0;

	if (rx > 0 || ry > 0)
	{
		_SDL_SVG_MoveTo (c, x + rx, y);
		_SDL_SVG_LineTo (c, x + width - rx, y);
		_SDL_SVG_ArcTo  (c, rx, ry, 0, 0, 1, x + width, y + ry);
		_SDL_SVG_LineTo (c, x + width, y + height - ry);
		_SDL_SVG_ArcTo  (c, rx, ry, 0, 0, 1, x + width - rx, y + height);
		_SDL_SVG_LineTo (c, x + rx, y + height);
		_SDL_SVG_ArcTo  (c, rx, ry, 0, 0, 1, x, y + height - ry);
		_SDL_SVG_LineTo (c, x, y + ry);
		_SDL_SVG_ArcTo  (c, rx, ry, 0, 0, 1, x + rx, y);
	}
	else
	{
		_SDL_SVG_MoveTo (c, x, y);
		_SDL_SVG_LineTo (c, x + width, y);
		_SDL_SVG_LineTo (c, x + width, y + height);
		_SDL_SVG_LineTo (c, x, y + height);
	}
	_SDL_SVG_ClosePath (c);

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
SDL_svg_context *c = closure;
double vpar, svgar;
double logic_width, logic_height;
double logic_x, logic_y;
double phys_width, phys_height;

	dprintf("svg_ApplyViewBox\n");

	phys_width = ConvertLength(width);
	phys_height = ConvertLength(height);

	vpar = view_box.box.width / view_box.box.height;
	svgar = phys_width / phys_height;
	logic_x = view_box.box.x;
	logic_y = view_box.box.y;
	logic_width = view_box.box.width;
	logic_height = view_box.box.height;

	if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_NONE)
	{
		_scale (c,
			 phys_width / logic_width,
			 phys_height / logic_height);
		_translate (c, -logic_x, -logic_y);
	} else if((vpar < svgar &&
			view_box.meet_or_slice == SVG_MEET_OR_SLICE_MEET) ||
			(vpar >= svgar &&
			view_box.meet_or_slice == SVG_MEET_OR_SLICE_SLICE))
	{
		_scale (c,
			phys_height / logic_height, phys_height / logic_height);

		if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMIN ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMAX)
			_translate (c, -logic_x, -logic_y);
		else if(view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX)
			_translate (c,
				 -logic_x - (logic_width - phys_width * logic_height / phys_height) / 2,
				 -logic_y);
		else
			_translate (c,
			 -logic_x - (logic_width - phys_width * logic_height / phys_height),
			 -logic_y);
	} else
	{
		_scale (c, phys_width / logic_width, phys_width / logic_width);

		if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMIN ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN)
			_translate (c, -logic_x, -logic_y);
		else if(view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMAXYMID)
			_translate (c,
				 -logic_x,
				 -logic_y - (logic_height - phys_height * logic_width / phys_width) / 2);
		else
			_translate (c,
			 -logic_x,
			 -logic_y - (logic_height - phys_height * logic_width / phys_width));
	}



	return SVG_STATUS_SUCCESS;
}

/* The ellipse and arc functions below are:
 
   Copyright (C) 2000 Eazel, Inc.
  
   Author: Raph Levien <raph@artofcode.com>

   This is adapted from svg-path in Gill.
*/
static void
_svg_path_arc_segment (SDL_svg_context *c,
                 double xc, double yc,
		       double th0, double th1,
		       double rx, double ry, double x_axis_rotation)
{
	double sin_th, cos_th;
	double a00, a01, a10, a11;
	double x1, y1, x2, y2, x3, y3;
	double t;
	double th_half;

	sin_th = sin (x_axis_rotation * (M_PI / 180.0));
	cos_th = cos (x_axis_rotation * (M_PI / 180.0)); 
	/* inverse transform compared with rsvg_path_arc */
	a00 = cos_th * rx;
	a01 = -sin_th * ry;
	a10 = sin_th * rx;
	a11 = cos_th * ry;

	th_half = 0.5 * (th1 - th0);
	t = (8.0 / 3.0) * sin (th_half * 0.5) * sin (th_half * 0.5) / sin (th_half);
	x1 = xc + cos (th0) - t * sin (th0);
	y1 = yc + sin (th0) + t * cos (th0);
	x3 = xc + cos (th1);
	y3 = yc + sin (th1);
	x2 = x3 + t * sin (th1);
	y2 = y3 - t * cos (th1);

	_SDL_SVG_CurveTo (c, a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
		a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
		a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}


/**
 * rx: Radius in x direction (before rotation).
 * ry: Radius in y direction (before rotation).
 * x_axis_rotation: Rotation angle for axes.
 * large_arc_flag: 0 for arc length <= 180, 1 for arc >= 180.
 * sweep: 0 for "negative angle", 1 for "positive angle".
 * x: New x coordinate.
 * y: New y coordinate.
 *
 **/
static svg_status_t _SDL_SVG_ArcTo (void *closure,
                                    double rx,
                                    double ry,
                                    double x_axis_rotation,
                                    int    large_arc_flag,
                                    int    sweep_flag,
                                    double x,
                                    double y)
{
SDL_svg_context *c=closure;
double sin_th, cos_th;
double a00, a01, a10, a11;
double x0, y0, x1, y1, xc, yc;
double d, sfactor, sfactor_sq;
double th0, th1, th_arc;
int i, n_segs;
double dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;
double curx, cury;

	dprintf("svg_ArcTo\n");

	rx = fabs (rx);
	ry = fabs (ry);

	curx = c->at.x;
	cury = c->at.y;

	sin_th = sin (x_axis_rotation * (M_PI / 180.0));
	cos_th = cos (x_axis_rotation * (M_PI / 180.0));

	dx = (curx - x) / 2.0;
	dy = (cury - y) / 2.0;
	dx1 =  cos_th * dx + sin_th * dy;
	dy1 = -sin_th * dx + cos_th * dy;
	Pr1 = rx * rx;
	Pr2 = ry * ry;
	Px = dx1 * dx1;
	Py = dy1 * dy1;
	/* Spec : check if radii are large enough */
	check = Px / Pr1 + Py / Pr2;
	if(check > 1)
	{
		rx = rx * sqrt(check);
		ry = ry * sqrt(check);
	}

	a00 = cos_th / rx;
	a01 = sin_th / rx;
	a10 = -sin_th / ry;
	a11 = cos_th / ry;
	x0 = a00 * curx + a01 * cury;
	y0 = a10 * curx + a11 * cury;
	x1 = a00 * x + a01 * y;
	y1 = a10 * x + a11 * y;
	/* (x0, y0) is current point in transformed coordinate space.
	   (x1, y1) is new point in transformed coordinate space.
	   
	   The arc fits a unit-radius circle in this space.
	*/
	d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
	sfactor_sq = 1.0 / d - 0.25;
	if (sfactor_sq < 0) sfactor_sq = 0;
	sfactor = sqrt (sfactor_sq);
	if (sweep_flag == large_arc_flag) sfactor = -sfactor;
	xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
	yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
	/* (xc, yc) is center of the circle. */
	
	th0 = atan2 (y0 - yc, x0 - xc);
	th1 = atan2 (y1 - yc, x1 - xc);
	
	th_arc = th1 - th0;
	if (th_arc < 0 && sweep_flag)
	th_arc += 2 * M_PI;
	else if (th_arc > 0 && !sweep_flag)
	th_arc -= 2 * M_PI;

	/* XXX: I still need to evaluate the math performed in this
	   function. The critical behavior desired is that the arc must be
	   approximated within an arbitrary error tolerance, (which the
	   user should be able to specify as well). I don't yet know the
	   bounds of the error from the following computation of
	   n_segs. Plus the "+ 0.001" looks just plain fishy. -cworth */
	n_segs = ceil (fabs (th_arc / (M_PI * 0.5 + 0.001)));
	
	for (i = 0; i < n_segs; i++) {
		_svg_path_arc_segment (c, xc, yc,
					th0 + i * th_arc / n_segs,
					th0 + (i + 1) * th_arc / n_segs,
					rx, ry, x_axis_rotation);
	}
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
	case SVG_LENGTH_UNIT_PCT:f=0.01;break;
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
		c->clip_xmin = 0;
		c->clip_ymin = 0;
		c->clip_xmax = 0;
		c->clip_ymax = 0;
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
	if(c->tags)
	{
		free(c->tags);
		c->tags=0;
	}
	if(c->pathstops)
	{
		free(c->pathstops);
		c->pathstops=0;
	}
	free(c);
}

int SVG_Version(void)
{
	return SVG_VERSION;
}

void SVG_Set_Flags(SDL_svg_context *c, unsigned long flags)
{
	c->flags = flags;
}

void SVG_SetClipping(SDL_svg_context *c, int xmin, int ymin,
	int xmax, int ymax)
{
	c->clip_xmin = xmin;
	c->clip_ymin = ymin;
	c->clip_xmax = xmax;
	c->clip_ymax = ymax;
	c->internal_flags |= INT_FLAG_CLIPPING_SET;
}

