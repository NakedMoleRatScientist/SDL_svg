#include <SDL.h>
#include "SDL_svg.h"
#include "internals.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <libsvg/svg.h>
#include "ftgrays.h"

#define IFACTOR 64 // used to fix coords to the grays rendering engine format


static Uint32 maprgb(SDL_Surface *thescreen, int r,int g,int b)
{
	return SDL_MapRGB(thescreen->format,r,g,b);
}

static void colordot_8(SDL_Surface *surf, int x, int y, unsigned long c, int f2)
{
	Uint8 *p = (Uint8*)surf->pixels + y * surf->pitch + x;
	*p = (Uint8) c;
}

static void colordot_16(SDL_Surface *surf, int x, int y, Uint32 c, int f2)
{
int r1,g1,b1,r2,g2,b2;
SDL_PixelFormat *f;
int a;
	a=((f2+1) * (c>>24))>>8;
	if(a==0xff)
		*((unsigned short *)surf->pixels+y *
			surf->pitch/2+x)=c;
	else
	{
		unsigned short *p,t;
		int ai;
		p=((unsigned short *)surf->pixels)+y *
			surf->pitch/2+x;
		ai=a^255;
		t=*p;
		f=surf->format;

		r1 = c & f->Rmask;
		g1 = c & f->Gmask;
		b1 = c & f->Bmask;

		r2 = t & f->Rmask;
		g2 = t & f->Gmask;
		b2 = t & f->Bmask;

		*p = (((a * r1 + ai * r2) >> 8) & f->Rmask) |
			(((a * g1 + ai * g2) >> 8) & f->Gmask) |
			(((a * b1 + ai * b2) >> 8) & f->Bmask);

	}
}

static void colordot_32(SDL_Surface *surf, int x, int y, Uint32 c, int f2)
{
Uint32 a;
	a=((f2+1) * (c>>24))>>8;
	if(a==0xff)
		*((Uint32 *)(surf->pixels)+y * surf->pitch/4+x)=c;
	else
	{
		Uint32 *p, t;
		Uint32 ai;
		
		p=(Uint32 *)(surf->pixels)+y * surf->pitch/4+x;

		ai=a^255;
		t=*p;

		*p = ((a*(c&0xff) + ai*(t&0xff))>>8) |
			(((a*(c&0xff00) + ai*(t&0xff00))&0xff0000)>>8) |
			(((a*(c&0xff0000) + ai*(t&0xff0000))&0xff000000)>>8);
	}
}
static void colordot_32_composite(SDL_Surface *surf, int x, int y, Uint32 c, int f2)
{
Uint32 a1, a2, a1a2, a1_a1a2, newa, newpixel;
Uint32 *p, t;
int c1, c2;

	a2 = ((f2+1) * (c>>24))>>8;

	if(a2==0xff)
		*((Uint32 *)surf->pixels+y *
			surf->pitch/4+x)=c | 0xff000000;
	else
	{
		p=((Uint32 *)surf->pixels)+y *
			surf->pitch/4+x;
		t=*p;
		a1=t>>24;
		a1a2=a1*a2/255;
		a1_a1a2=a1-a1a2;
		newa = a2+a1_a1a2;

		newpixel = newa<<24;
		c1=255 & (t>>16);
		c2=255 & (c>>16);
		newpixel |= ((c1*a1_a1a2 + c2*a2) / newa) << 16;
		c1=255 & (t>>8);
		c2=255 & (c>>8);
		newpixel |= ((c1*a1_a1a2 + c2*a2) / newa) << 8;
		c1=255 & t;
		c2=255 & c;
		*p = newpixel | (c1*a1_a1a2 + c2*a2) / newa;
	}
}

static void lineargradient(SDL_svg_context *c, void *_span, int y)
{
float cx,cy;
float vx,vy;
float f;
float dx,dy;
int dp;
int policy;
int x, w, coverage;


	x=((FT_Span *)_span)->x;
	w=((FT_Span *)_span)->len;
	coverage=((FT_Span *)_span)->coverage;

	policy = c->gradient_policy;


	cx=c->gradient_p1.x;
	cy=c->gradient_p1.y;

	vx=c->gradient_p2.x - cx;
	vy=c->gradient_p2.y - cy;

	f=(vx*vx+vy*vy);

	dx = c->gm.a * x + c->gm.c * y + c->gm.e - cx;
	dy = c->gm.b * x + c->gm.d * y + c->gm.f - cy;

	while(w--)
	{

		dp=(NUM_GRADIENT_COLORS-1) *(dx*vx + dy*vy)/f;
		if(policy == SVG_GRADIENT_SPREAD_PAD)
		{
			if(dp<0) dp=0;
			if(dp>NUM_GRADIENT_COLORS-1) dp=NUM_GRADIENT_COLORS-1;
		} else if(policy == SVG_GRADIENT_SPREAD_REPEAT)
		{
			dp&=(NUM_GRADIENT_COLORS-1);
		} else // SVG_GRADIENT_SPREAD_REFLECT
		{
			if(dp&NUM_GRADIENT_COLORS)
				dp^=(NUM_GRADIENT_COLORS-1);
			dp&=(NUM_GRADIENT_COLORS-1);
		}
		c->colordot(c->surface, x++, y, c->gradient_colors[dp], coverage);
		dx+=c->gm.a;
		dy+=c->gm.b;
	}
}


/*
P1 = circle center or radius r
P  = point on the texture whose color we want to determine
P2 = point of focus for lighting
The line along P2 to P intersects the circle at 2 places
Using vector arithmetic work out the solution to
MAGNITUDE ( P2 + s * (P-P2) - P1 ) = r
The value of s can be used to determine where along the line of P2 to
the circle P is. At s==1.0 we're at P.
It works out to solving the quadratic equation. We get a singularity
at P==P2.
If P2 == P1 we have
s = r / MAGNITUDE ( P - P1 )
also with a singularity at P == P1

We're really trying to determine the fraction along the line of P2 to P
to the circle P is, 0 <= fraction <= 1.0.

(DA) 20041220
*/

static void radialgradient(SDL_svg_context *c, void *_span, int y)
{
int policy;
float va,vb,vc;
float dx,dy;
float x1c,y1c;
float v;
int dp;
float cx,cy;
float r,r2;
int x, w, coverage;

	x=((FT_Span *)_span)->x;
	w=((FT_Span *)_span)->len;
	coverage=((FT_Span *)_span)->coverage;

	policy = c->gradient_policy;
	cx=c->gradient_p1.x;
	cy=c->gradient_p1.y;
	r=c->gradient_r;

	r2=r * r;
	x1c=c->gradient_p2.x - cx;
	y1c=c->gradient_p2.y - cy;
	vc=x1c*x1c + y1c*y1c - r2;

	dx=c->gm.a * x + c->gm.c * y + c->gm.e - c->gradient_p2.x;
	dy=c->gm.b * x + c->gm.d * y + c->gm.f - c->gradient_p2.y;
//printf("%f %f %f %f\n", dx, dy, c->gradient_p2.x, c->gradient_p2.y);
	while(w--)
	{
		va=dx*dx+dy*dy;
		vb=2*(x1c*dx + y1c*dy);
		if(dy || dx)
		{
			v=sqrt(vb*vb-4*va*vc);
			dp=510.0*va/(-vb + v);
		} else
			dp=0;
		if(policy == SVG_GRADIENT_SPREAD_PAD)
		{
			if(dp<0) dp=0;
			if(dp>NUM_GRADIENT_COLORS-1) dp=NUM_GRADIENT_COLORS-1;
		} else if(policy == SVG_GRADIENT_SPREAD_REPEAT)
		{
			dp&=(NUM_GRADIENT_COLORS-1);
		} else // SVG_GRADIENT_SPREAD_REFLECT
		{
			if(dp&NUM_GRADIENT_COLORS)
				dp^=(NUM_GRADIENT_COLORS-1);
			dp&=(NUM_GRADIENT_COLORS-1);
		}
		c->colordot(c->surface, x++, y, c->gradient_colors[dp], coverage);
		dx += c->gm.a;
		dy += c->gm.b;
	}

}

static void solidstrip(SDL_svg_context *c, void *_span, int y)
{
int x, w, coverage;

	x=((FT_Span *)_span)->x;
	w=((FT_Span *)_span)->len;
	coverage=((FT_Span *)_span)->coverage;
	while(w--)
		c->colordot(c->surface, x++, y, c->solidcolor, coverage);
}

void myspanner(int y, int count, FT_Span *spans, void *user)
{
SDL_svg_context *c=user;

// renders spans in reverse order, but why not? They don't overlap
	while(count--)
		c->renderfunc(c, spans+count, y);
}

static void do_render(SDL_svg_context *c)
{
FT_Vector *points;
int res;
FT_Raster myraster;
FT_Raster_Params myparams;
FT_Outline myoutline;
int n;
int i;
int iscomposite;


	if(!c->renderfunc) return;

	iscomposite = c->flags & SDL_SVG_FLAG_COMPOSITE;

	if(c->surface->format->BytesPerPixel == 1)
	{
		c->colordot = colordot_8;
	} else if(c->surface->format->BytesPerPixel == 2)
	{
		c->colordot = colordot_16;
	} else
	{
		if(!iscomposite)
			c->colordot = colordot_32;
		else
			c->colordot = colordot_32_composite;
	}

	n = c->numpoints;

	if(!n) return;

	points = malloc(n*sizeof(FT_Vector));
	for(i=0;i<n;++i)
	{
		points[i].x = c->path[i].x * IFACTOR;
		points[i].y = c->path[i].y * IFACTOR;
	}

	myoutline.n_contours = c->numpathstops;
	myoutline.n_points = c->numpoints;
	myoutline.points = points;
	myoutline.tags = c->tags;
	myoutline.contours = c->pathstops;
	myoutline.flags = FT_OUTLINE_IGNORE_DROPOUTS |
		((c->fill_rule == SVG_FILL_RULE_EVEN_ODD) ?
			FT_OUTLINE_EVEN_ODD_FILL : 0);

	myparams.target = 0;
	myparams.source = &myoutline;
	myparams.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA |
		FT_RASTER_FLAG_CLIP;
	myparams.gray_spans = myspanner;
	myparams.user = c;
	if(c->internal_flags & INT_FLAG_CLIPPING_SET)
	{
		myparams.clip_box.xMin = c->clip_xmin;
		myparams.clip_box.yMin = c->clip_ymin;
		myparams.clip_box.xMax = c->clip_xmax;
		myparams.clip_box.yMax = c->clip_ymax;
	} else
	{
		myparams.clip_box.xMin = 0;
		myparams.clip_box.xMax = c->surface->w;
		myparams.clip_box.yMin = 0;
		myparams.clip_box.yMax = c->surface->h;
	}

// WARNING: The  grays raster_new is not thread safe, all instances use the
// same raster structure

	res=svg_ft_grays_raster.raster_new(0, &myraster);
	svg_ft_grays_raster.raster_reset(myraster, c->pool, sizeof(c->pool));

	res=svg_ft_grays_raster.raster_render(myraster, &myparams);

	svg_ft_grays_raster.raster_done(myraster);
	free(points);
}



void svg_render_solid(SDL_svg_context *c)
{
int i,j;
int colorstops;
//void (*renderfunc)(SDL_svg_context *c, int x, int y, int w);
IPoint *path;
svg_paint_t *paint;
const svg_color_t *rgb;
int alpha;
svg_matrix_t tm;

	c->renderfunc = 0;
	path = c->path;

	paint = c->paint;
	switch(paint->type)
	{
	case SVG_PAINT_TYPE_COLOR:
		rgb = &paint->p.color;
		alpha = 255.0 * c->FillOpacity;
		c->solidcolor = maprgb(c->surface,
			svg_color_get_red(rgb),
			svg_color_get_green(rgb),
			svg_color_get_blue(rgb)) |
			(alpha << 24);
		c->renderfunc = solidstrip;
		break;
	case SVG_PAINT_TYPE_GRADIENT:
		colorstops = paint->p.gradient->num_stops;
		for(i=0;i<colorstops-1;++i)
		{
			int c1,c2;
			int r1,g1,b1,a1,r2,g2,b2,a2;
			Uint32 t;
			int v;

			c1=NUM_GRADIENT_COLORS*i/(colorstops-1);
			c2=NUM_GRADIENT_COLORS*(i+1)/(colorstops-1);

			t=paint->p.gradient->stops[i].color.rgb;
			a1=255.0 * paint->p.gradient->stops[i].opacity;
			r1 = (t>>16) & 255;
			g1 = (t>>8) & 255;
			b1 = t & 255;

			t=paint->p.gradient->stops[i+1].color.rgb;
			a2=255.0 * paint->p.gradient->stops[i+1].opacity;
			r2 = (t>>16) & 255;
			g2 = (t>>8) & 255;
			b2 = t & 255;

			r2-=r1;
			g2-=g1;
			b2-=b1;
			a2-=a1;

			v=c2-c1-1;

			for(j=0;j<=v;++j)
			{
				c->gradient_colors[c1+j]=maprgb(c->surface, 
					r1 + r2*j/v,
					g1 + g2*j/v,
					b1 + b2*j/v) | ((a1 + a2*j/v)<<24);
			}
		}

		c->gradient_policy = paint->p.gradient->spread;
		if (paint->p.gradient->type == SVG_GRADIENT_LINEAR)
		{
			c->renderfunc = lineargradient;
			c->gradient_p1 = (IPoint)
				{ ConvertLength(&paint->p.gradient->u.linear.x1),
				  ConvertLength(&paint->p.gradient->u.linear.y1)};
			c->gradient_p2 = (IPoint)
				{ ConvertLength(&paint->p.gradient->u.linear.x2),
				  ConvertLength(&paint->p.gradient->u.linear.y2)};
		} else if(paint->p.gradient->type == SVG_GRADIENT_RADIAL)
		{
			c->renderfunc = radialgradient;
			c->gradient_p1 = (IPoint)
				{ ConvertLength(&paint->p.gradient->u.radial.cx),
				  ConvertLength(&paint->p.gradient->u.radial.cy)};
			c->gradient_p2 = (IPoint)
				{ ConvertLength(&paint->p.gradient->u.radial.fx),
				  ConvertLength(&paint->p.gradient->u.radial.fy)};
			c->gradient_r = ConvertLength(&paint->p.gradient->u.radial.r);
		} else c->renderfunc = 0;

		if(paint->p.gradient->units==SVG_GRADIENT_UNITS_USER)
		{
			IPoint tp;
			tp = FixCoords(c, (IPoint) {c->w, c->h});

			c->gm.a = c->w / tp.x;
			c->gm.d = c->h / tp.y;
			c->gm.c = c->gm.b = c->gm.e = c->gm.f = 0.0;

		} else // BBOX
		{
			IPoint vx, vy, tv;
			float xx,xy,yx,yy, det;
			tv = FixCoords(c, (IPoint) {c->minx, c->miny});
			vx = FixCoords(c, (IPoint) {c->maxx, c->miny});
			vy = FixCoords(c, (IPoint) {c->minx, c->maxy});

// x' = x+e
// y' = y+f
// x'' = ax' + by'
// y'' = cx' + dy'

			xx = vx.x - tv.x;
			xy = vx.y - tv.y;
			yx = vy.x - tv.x;
			yy = vy.y - tv.y;
			det = xx*yy - xy*yx;
			if(det != 0.0)
			{
				c->gm.a = yy / det;
				c->gm.c = -yx / det;
				c->gm.b = -xy / det;
				c->gm.d = xx / det;
			} else
				c->gm.a = c->gm.b = c->gm.c = c->gm.d = 0.0;
			c->gm.e = -(c->gm.a * tv.x + c->gm.c * tv.y);
			c->gm.f = -(c->gm.b * tv.x + c->gm.d * tv.y);
		}
// go get the gradient transform
		svg_matrix_init(&tm,
			paint->p.gradient->transform[0],
			paint->p.gradient->transform[1],
			paint->p.gradient->transform[2],
			paint->p.gradient->transform[3],
			paint->p.gradient->transform[4],
			paint->p.gradient->transform[5]);
// invert it
		tm = svg_matrix_invert(&tm);
// apply it to our own gm
		svg_matrix_multiply(&c->gm, &tm, &c->gm);

		break;
	default:
		c->renderfunc = 0;
		break;
	}

	do_render(c);

}
