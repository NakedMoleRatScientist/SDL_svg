#include <SDL.h>
#include "SDL_svg.h"
#include "internals.h"
#include <math.h>
#include <svg.h>

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

#define FACTOR 16
static inline void acolordot(SDL_Surface *s, int x, int y, unsigned long c, int f2)
{
unsigned long a;
	a=f2 * ((c&0xff000000)/FACTOR);
	colordot(s, x, y, (a&0xff000000) | (c&0xffffff));
}

#define MAX_SPANS_PER_ROW 32
struct spanlist
{
	int havespan;
	int x;
	int runs[MAX_SPANS_PER_ROW*2+2];
};

#define MAXLINES 2048
struct spantab
{
	int miny, maxy;
	int x;
	int firsty;
	int secondy;
	int lasty;
	int lastlasty;
	struct spanlist spanlists[MAXLINES];
};

static void initspantab(struct spantab *spantab)
{
int i;
struct spanlist *sl;

	for(i=0;i<MAXLINES;++i)
	{
		sl=spantab->spanlists+i;
		sl->havespan=0;
		sl->runs[0]=1000000;
		sl->runs[1]=0;
		sl->runs[2]=0;
	}
	spantab->miny=0x7fffffff;
	spantab->maxy=-0x7fffffff;
}

#define INVALID_Y -888888

// call after initspantab() at start of path
static void preparespantab(struct spantab *spantab, int x, int ys, int ye)
{
	spantab->x=x;
	spantab->firsty = ys;
	spantab->lastlasty=INVALID_Y;
	spantab->lasty=ys >=0 ? ys : INVALID_Y;
	spantab->secondy=INVALID_Y;
}

static void mergespans(struct spantab *spantab, int *put, int *take,
		int minx, int maxx, int delta)
{
int x=0;
int len;
int newlen;
int t;
int *oput;
int type;

	oput=put;
	++maxx;
	while((len = *take++))
	{
		type=*take++;
		if(minx >= x+len || maxx<=x)
		{
			x+=len;
			*put++ = len;
			*put++ = type;
			continue;
		}
		if(minx>x)
		{
			t=minx-x;
			*put++ = t;
			*put++ = type;
			len -= t;
			x = minx;
		}
		newlen = maxx - x;
		if(newlen>len) newlen=len;
		*put++ = newlen;
		*put++ = type + delta;
		minx += newlen;
		x+=newlen;
		if(len>newlen)
		{
			t = len - newlen;
			*put++ = t;
			*put++ = type;
			x += t;
			minx += t;
		}
	}
	*put=0;
#if 0 // merge adjascent spans that have same value. Probably not worth doing
	put=oput;
	take=oput+2;
	while(*put)
	{
		if(*take && (put[1]==take[1]))
		{
			*put += *take;
		} else
		{
			put[2] = *take;
			put[3] = take[1];
			put+=2;
		}
		take+=2;
	}
	*put=0;
#endif
}

static void newcrossing(struct spantab *spantab)
{
struct spanlist *sl;
int v;
int runs[MAX_SPANS_PER_ROW*2+2];
int minx, maxx;
int *put, *take;

	if(spantab->lasty<0 || spantab->lasty>=MAXLINES) return;
	sl=spantab->spanlists + spantab->lasty;

	if(!sl->havespan)
	{
		sl->x = spantab->x;
		sl->havespan=1;
		return;
	}
	sl->havespan = 0;

	minx=sl->x;
	maxx=spantab->x;
	if(minx > maxx)
	{
		v=minx;
		minx=maxx;
		maxx=v;
		v=-1;
	} else
		v=1;

	if(spantab->lasty>spantab->firsty) v=-v;

	put=runs;
	take=sl->runs;

	mergespans(spantab, put, take, minx, maxx, v);

// copy back into runs
	for(;;)
	{
		v = *take++ = *put++;
		if(!v) break;
		*take++ = *put++;
	}
}

static void adddot(struct spantab *spantab, int x, int y)
{
	if(y>=0 && y<=MAXLINES)
	{
		if(y<spantab->miny) spantab->miny=y;
		if(y>spantab->maxy) spantab->maxy=y;
	}
	if(y==spantab->lasty)
	{
		spantab->x=x;
	} else
	{
		if(spantab->secondy==INVALID_Y)
		{
			spantab->secondy=y;
			if(spantab->secondy > spantab->firsty)
				--spantab->firsty;
		}
		if(spantab->lasty!=INVALID_Y)
		{
			if(y!=spantab->lastlasty)
				newcrossing(spantab);
			else
			{
				int t;
				t=spantab->x;
				newcrossing(spantab);
				spantab->x = t;
				newcrossing(spantab);
			}
		}
		spantab->lastlasty=spantab->lasty;
		spantab->lasty=y;
		spantab->x=x;
	}
}

static void v2(struct spantab *spantab, int x1,int y1,int x2,int y2)
{
#define FRAC 12
int yn;
int dx,dy;
	yn=y2-y1;
	if(yn<0) yn=-yn;
	if(!yn) return;
	adddot(spantab, x1, y1);
	dy=y2>y1 ? 1 : (y2<y1 ? -1 : 0);
	dx = ((x2-x1)<<FRAC) / (yn+1);
	x1<<=FRAC;
	while(yn--)
	{
		y1+=dy;
		x1+=dx;
		adddot(spantab, x1>>FRAC, y1);
	}
}

static void generatespans(struct spantab *spantab,
		SDL_svg_context *c, IPoint *points, int num)
{
int i,j;
int n2;

	if(!num) return;
	n2=num;
	while(--n2>0)
	{
		if((int) points[0].y != (int) points[n2].y)
			break;
	}
	preparespantab(spantab, (int) (points[0].x*FACTOR), (int) points[0].y,
		(int)points[n2].y);
	for(i=0;i<num;++i)
	{
		j=i+1;
		if(j==num)
			j=0;
		v2(spantab, (int)(points[i].x*FACTOR), (int) points[i].y,
			(int) (points[j].x*FACTOR), (int) points[j].y);
	}
	if(spantab->secondy==spantab->lastlasty)
		newcrossing(spantab);
}
static void renderspans(struct spantab *spantab,
		void renderfunc(SDL_svg_context *arg, int x, int y, int w),
		SDL_svg_context *c)
{
int y;
int fill_rule;
int len;
int *take;
int minx;

	fill_rule = (c->fill_rule==SVG_FILL_RULE_NONZERO) ? ~0 : 1;

	for(y=spantab->miny; y<=spantab->maxy;++y)
	{

		take=spantab->spanlists[y].runs;
		minx=0;
		while((len=*take))
		{
			if(take[1] & fill_rule)
				renderfunc(c, minx, y, len);
			minx+=len;
			take+=2;
		}
		
	}
}




static void lineargradient(SDL_svg_context *c, int ox, int y, int w)
{
float cx,cy;
float vx,vy;
float f;
float dx,dy,dyvy;
int dp;
int policy;
int f2;
int ex,exf;
int x;

	ex=ox+w;
	exf=ex/FACTOR;
	f2 = FACTOR - (ox&(FACTOR-1));
	x=ox/FACTOR;
	policy = c->gradient_policy;


	cx=c->gradient_p1.x;
	cy=c->gradient_p1.y;

	vx=c->gradient_p2.x - cx;
	vy=c->gradient_p2.y - cy;

	f=vx*vx+vy*vy;

	dy=y-cy;
	dyvy = dy*vy;

	while(x<=exf)
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
		if(f2)
		{
			acolordot(c->surface, x++, y, c->gradient_colors[dp], f2);
			f2=0;
		} else
		{
			if(x<exf)
				colordot(c->surface, x++, y, c->gradient_colors[dp]);
			else
			{
				w = ex & (FACTOR-1);
				if(w)
			 		acolordot(c->surface, x, y, c->gradient_colors[dp], w);
				break;
			}
		}
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

static void radialgradient(SDL_svg_context *c, int ox, int y, int w)
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
int f2;
int ex,exf;
int x;

	ex=ox+w;
	exf=ex/FACTOR;
	f2 = FACTOR - (ox&(FACTOR-1));
	x=ox/FACTOR;
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
	while(x<=exf)
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
		if(f2)
		{
			acolordot(c->surface, x++, y, c->gradient_colors[dp], f2);
			f2=0;
		} else
		{
			if(x<exf)
				colordot(c->surface, x++, y, c->gradient_colors[dp]);
			else
			{
				w = ex & (FACTOR-1);
				if(w)
			 		acolordot(c->surface, x, y, c->gradient_colors[dp], w);
				break;
			}
		}
	}

}

static void solidstrip(SDL_svg_context *c, int ox, int y, int w)
{
int f2;
int x;
int ex,exf;
	ex=ox + w;
	exf = ex/FACTOR;
	f2 = FACTOR - (ox&(FACTOR-1));
	x=ox/FACTOR;

	while(x < exf)
	{
		if(f2)
		{
			acolordot(c->surface, x++, y, c->solidcolor, f2);
			f2=0;
		}
		colordot(c->surface, x++, y, c->solidcolor);
	}
	w = ex & (FACTOR-1);
	if(w)
		acolordot(c->surface, x, y, c->solidcolor, w);
}

void solid(SDL_svg_context *c)
{
int i,j,k;
int colorstops;
void (*renderfunc)(SDL_svg_context *c, int x, int y, int w);
float minx, miny, maxx, maxy;
IPoint *ip;
IPoint *path;
int n;
int startnum;
struct spantab spans;
svg_paint_t *paint;
const svg_color_t *rgb;
int alpha;

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
		renderfunc = solidstrip;
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
			renderfunc = lineargradient;
			c->gradient_p1 = (IPoint)
				{ paint->p.gradient->u.linear.x1.value,
				  paint->p.gradient->u.linear.y1.value};
			c->gradient_p2 = (IPoint)
				{ paint->p.gradient->u.linear.x2.value,
				  paint->p.gradient->u.linear.y2.value};
		} else if(paint->p.gradient->type == SVG_GRADIENT_RADIAL)
		{
			renderfunc = radialgradient;
			c->gradient_p1 = (IPoint)
				{ paint->p.gradient->u.radial.cx.value,
				  paint->p.gradient->u.radial.cy.value};
			c->gradient_p2 = (IPoint)
				{ paint->p.gradient->u.radial.fx.value,
				  paint->p.gradient->u.radial.fy.value};
			c->gradient_r = paint->p.gradient->u.radial.r.value;
		} else renderfunc = 0;

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
		renderfunc = 0;
		break;
	}

	initspantab(&spans);

	for(k=startnum=0;k<c->numpathstops;++k)
	{
		n=c->pathstops[k];

		generatespans(&spans, c, path+startnum, n-startnum);
		startnum = n;

	}
	if(renderfunc)
		renderspans(&spans, renderfunc, c);
}
