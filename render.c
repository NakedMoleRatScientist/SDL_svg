#include <SDL.h>
#include "SDL_svg.h"
#include "internals.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <svg.h>
#include "ftgrays.h"

#define IFACTOR 64 // used to fix coords to the grays rendering engine format


static unsigned long maprgb(SDL_Surface *thescreen, int r,int g,int b)
{
	return SDL_MapRGB(thescreen->format,r,g,b);
}

static void colordot(void *arg, int x, int y, unsigned long c)
{
	if(x<0 || y<0 ||
		x>=((SDL_Surface *)arg)->w ||
		y>=((SDL_Surface *)arg)->h)
		return;
	if((c>>24)==0xff)
		*((unsigned long *)((SDL_Surface *)arg)->pixels+y *
			((SDL_Surface *)arg)->pitch/4+x)=c;
	else
	{
		unsigned long *p,t;
		unsigned long a,ai;
		p=((unsigned long *)((SDL_Surface *)arg)->pixels)+y *
			((SDL_Surface *)arg)->pitch/4+x;
		a=c>>24;
		ai=a^255;
		t=*p;

		*p = ((a*(c&0xff) + ai*(t&0xff))>>8) |
			(((a*(c&0xff00) + ai*(t&0xff00))&0xff0000)>>8) |
			(((a*(c&0xff0000) + ai*(t&0xff0000))&0xff000000)>>8);
	}
}

static inline void acolordot(SDL_Surface *s, int x, int y, unsigned long c, int f2)
{
unsigned long a;
	a=(f2+1) * ((c&0xff000000)>>8);
	colordot(s, x, y, (a&0xff000000) | (c&0xffffff));
}


static void lineargradient(SDL_svg_context *c, void *_span, int y)
{
float cx,cy;
float vx,vy;
float f;
float dx,dy,dyvy;
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

	f=vx*vx+vy*vy;

	dy=y-cy;
	dyvy = dy*vy;

	while(w--)
	{
		dx = x - cx;
		dp=(NUM_GRADIENT_COLORS-1) *(dx*vx + dyvy)/f;
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
		acolordot(c->surface, x++, y, c->gradient_colors[dp], coverage);
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
float dy2,y1cdy;
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
	dy=y - c->gradient_p2.y;
	dy2=dy * dy;
	y1cdy=y1c * dy;
	while(w--)
	{
		dx=x - c->gradient_p2.x;
		va=dx*dx+dy2;
		vb=2*(x1c*dx + y1cdy);
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
		acolordot(c->surface, x++, y, c->gradient_colors[dp], coverage);
	}

}

static void solidstrip(SDL_svg_context *c, void *_span, int y)
{
int x, w, coverage;

	x=((FT_Span *)_span)->x;
	w=((FT_Span *)_span)->len;
	coverage=((FT_Span *)_span)->coverage;
	while(w--)
		acolordot(c->surface, x++, y, c->solidcolor, coverage);
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

	if(!c->renderfunc) return;

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
	myoutline.flags = FT_OUTLINE_IGNORE_DROPOUTS;
	if(c->fill_rule == SVG_FILL_RULE_EVEN_ODD)
		myoutline.flags |= FT_OUTLINE_EVEN_ODD_FILL;

	myparams.target = 0;
	myparams.source = &myoutline;
	myparams.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA |
		FT_RASTER_FLAG_CLIP;
	myparams.gray_spans = myspanner;
	myparams.user = c;
	myparams.clip_box.xMin = 0;
	myparams.clip_box.xMax = IFACTOR * c->surface->w;
	myparams.clip_box.yMin = 0;
	myparams.clip_box.yMax = IFACTOR * c->surface->h;

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
float minx, miny, maxx, maxy;
IPoint *ip;
IPoint *path;
svg_paint_t *paint;
const svg_color_t *rgb;
int alpha;

	c->renderfunc = 0;
	path = c->path;

	minx=miny = 0x7fffffff;
	maxx=maxy =-0x7fffffff;

	for(i=0, ip=path;i<c->numpoints;++i,++ip)
	{
		if(ip->x<minx) minx=ip->x;
		if(ip->x>maxx) maxx=ip->x;
		if(ip->y<miny) miny=ip->y;
		if(ip->y>maxy) maxy=ip->y;
	}

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
			unsigned long t;
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
				{ paint->p.gradient->u.linear.x1.value,
				  paint->p.gradient->u.linear.y1.value};
			c->gradient_p2 = (IPoint)
				{ paint->p.gradient->u.linear.x2.value,
				  paint->p.gradient->u.linear.y2.value};
		} else if(paint->p.gradient->type == SVG_GRADIENT_RADIAL)
		{
			c->renderfunc = radialgradient;
			c->gradient_p1 = (IPoint)
				{ paint->p.gradient->u.radial.cx.value,
				  paint->p.gradient->u.radial.cy.value};
			c->gradient_p2 = (IPoint)
				{ paint->p.gradient->u.radial.fx.value,
				  paint->p.gradient->u.radial.fy.value};
			c->gradient_r = paint->p.gradient->u.radial.r.value;
		} else c->renderfunc = 0;

		if(paint->p.gradient->units==SVG_GRADIENT_UNITS_USER)
		{
			c->gradient_p1 = FixCoords(c, c->gradient_p1);
			c->gradient_p2 = FixCoords(c, c->gradient_p2);
			c->gradient_r = FixSizes(c, (IPoint) {c->gradient_r, 0.0}).x;
		} else // BBOX
		{
			c->gradient_p1 = (IPoint)
				{minx + c->gradient_p1.x * (maxx - minx + 1),
				miny + c->gradient_p1.y * (maxy - miny + 1)};
			c->gradient_p2 = (IPoint)
				{minx + c->gradient_p2.x * (maxx - minx + 1), 
				miny + c->gradient_p2.y * (maxy - miny + 1)};
			c->gradient_r = c->gradient_r * (maxx - minx + 1);
		}
		break;
	default:
		c->renderfunc = 0;
		break;
	}

	do_render(c);

}
