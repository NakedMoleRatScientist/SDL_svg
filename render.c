#include <SDL.h>
#include "SDL_svg.h"
#include <math.h>

unsigned long maprgb(SDL_Surface *thescreen, int r,int g,int b)
{
	return SDL_MapRGB(thescreen->format,r,g,b);
}

void colordot(void *arg, int x, int y, unsigned long c)
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
		int a,ai;
		p=((unsigned long *)((SDL_Surface *)arg)->pixels)+y *
			((SDL_Surface *)arg)->pitch/4+x;
		a=c>>24;
		ai=a^255;
		t=*p;

		*p = ((a*(c&255) + ai*(t&255))>>8) |
			(((a*((c>>8)&255) + ai*((t>>8)&255))>>8)<<8) |
			(((a*((c>>16)&255) + ai*((t>>16)&255))>>8)<<16);

	}
}

void vector(void (*func)(void *arg, int x, int y, unsigned long c), void *arg,
	int sx,int sy,int dx,int dy,int c)
{
int diffx,diffy;
int stepx,stepy;
float val,delta;

//	dprintf("vector %d,%d to %d,%d\n", sx, sy, dx, dy);
	stepx=stepy=0;
	diffx=dx-sx;
	if(diffx<0) {diffx=-diffx;stepx=-1;}
	else if(diffx>0) stepx=1;
	diffy=dy-sy;
	if(diffy<0) {diffy=-diffy;stepy=-1;}
	else if(diffy>0) stepy=1;

	func(arg, sx,sy,c);

	if(diffx>diffy)
	{
		val=sy;
		delta=(dy-val)/diffx;
		val+=0.5;
		while(diffx--)
		{
			sx+=stepx;
			val+=delta;
			func(arg, sx,(int)val,c);
		}
	} else if(diffx<diffy)
	{
		val=sx;
		delta=(dx-val)/diffy;
		val+=0.5;
		while(diffy--)
		{
			sy+=stepy;
			val+=delta;
			func(arg, (int)val,sy,c);
		}
	} else
		while(diffx--)
		{
			sx+=stepx;
			sy+=stepy;
			func(arg, sx,sy,c);
		}
}

struct span
{
	int xin;
	int xout;
};

struct spanlist
{
	int numspans;
	struct span spans[64];
};

#define MAXLINES 2048
struct spantab
{
	int miny, maxy;
	int xin, xout;
	int secondy;
	int lasty;
	int lastlasty;
	struct spanlist spanlists[MAXLINES];
};

void initspantab(struct spantab *spantab)
{
int i;
	spantab->miny=0;
	spantab->maxy=MAXLINES-1;
	for(i=spantab->miny;i<=spantab->maxy;++i)
		spantab->spanlists[i].numspans=0;
	spantab->miny=0x7fffffff;
	spantab->maxy=-0x7fffffff;
}

// call after initspantab()
void preparespantab(struct spantab *spantab, int x, int ys, int ye)
{
	spantab->xin=spantab->xout=x;
	spantab->lastlasty=-1;
	spantab->lasty=ys >=0 ? ys : -1;
	spantab->secondy=-1;
}

void adddot(struct spantab *spantab, int x, int y)
{
struct spanlist *sl;

	if(y<0 || y>=MAXLINES) return;
	if(y<spantab->miny) spantab->miny=y;
	if(y>spantab->maxy) spantab->maxy=y;
	if(y==spantab->lasty)
	{
		spantab->xout=x;
	} else
	{
		if(spantab->secondy<0) spantab->secondy=y;
		if(spantab->lasty>=0)
		{
			sl = spantab->spanlists+spantab->lasty;
			sl->spans[sl->numspans].xin = spantab->xin;
			sl->spans[sl->numspans].xout = spantab->xout;
			++sl->numspans; // WARNING bounds check!!!!!!
			if(y==spantab->lastlasty)
			{
				sl->spans[sl->numspans].xin = spantab->xin;
				sl->spans[sl->numspans].xout = spantab->xout;
				++sl->numspans; // WARNING bounds check!!!!!!
			}
		}
			spantab->lastlasty=spantab->lasty;
			spantab->lasty=y;
			spantab->xin=spantab->xout=x;
	}
}

void closelast(struct spantab *spantab, int x, int y)
{
struct spanlist *sl;
	if(spantab->secondy==spantab->lastlasty)
	{
		if(spantab->lasty<0 || spantab->lasty>=MAXLINES) return;
		sl=spantab->spanlists + spantab->lasty;
		sl->spans[sl->numspans].xin = spantab->xin;
		sl->spans[sl->numspans].xout = spantab->xout;
		++sl->numspans; // WARNING bounds check!!!!!!
	}
}

void v2(struct spantab *spantab, int sx,int sy,int dx,int dy)
{
int diffx,diffy;
int stepx,stepy;
float val,delta;

	stepx=stepy=0;
	diffx=dx-sx;
	if(diffx<0) {diffx=-diffx;stepx=-1;}
	else if(diffx>0) stepx=1;
	diffy=dy-sy;
	if(diffy<0) {diffy=-diffy;stepy=-1;}
	else if(diffy>0) stepy=1;

	adddot(spantab, sx, sy);

	if(diffx>diffy)
	{
		val=sy;
		delta=(dy-val)/diffx;
		val+=0.5;
		while(diffx--)
		{
			sx+=stepx;
			val+=delta;
			adddot(spantab, sx,(int)val);
		}
	} else if(diffx<diffy)
	{
		val=sx;
		delta=(dx-val)/diffy;
		val+=0.5;
		while(diffy--)
		{
			sy+=stepy;
			val+=delta;
			adddot(spantab, (int)val,sy);
		}
	} else
		while(diffx--)
		{
			sx+=stepx;
			sy+=stepy;
			adddot(spantab, sx,sy);
		}
}

void mergespans(struct spantab *spantab, int *put, int *take,
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

void generatespans(struct spantab *spantab,
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
	preparespantab(spantab, (int) points[0].x, (int) points[0].y, (int)points[n2].y);
	for(i=0;i<num;++i)
	{
		j=i+1;
		if(j==num)
			j=0;
		v2(spantab, (int)points[i].x, (int) points[i].y,
			(int) points[j].x, (int) points[j].y);
	}
	closelast(spantab, points[num-1].x, points[num-1].y);
}
void renderspans(struct spantab *spantab,
		void renderfunc(SDL_svg_context *arg, int x, int y, int w),
		SDL_svg_context *c)
{
int y;
int fill_rule;
struct spanlist *sl;
int i,j;

	fill_rule = (c->fill_rule==SVG_FILL_RULE_NONZERO) ? ~0 : 1;

	for(y=spantab->miny; y<=spantab->maxy;++y)
	{
		int runs1[2048],runs2[2048];
		int ping;
		int *take, *put;
		int minx, maxx;

		ping=0;
		runs1[0]=1000000;
		runs1[1]=0;
		runs1[2]=0;

		sl=spantab->spanlists+y;

		for(i=0;i<sl->numspans-1;i+=2)
		{
			struct span *s1, *s2;
			int v;
			s1=sl->spans+i;
			s2=sl->spans+i+1;

			minx=s1->xin;
			if(s1->xout<minx) minx=s1->xout;
			maxx=s2->xin;
			if(s2->xout>maxx) maxx=s2->xout;
			if(minx > maxx) v=-1;
			else v=1;
			if(s2->xin<minx) minx=s2->xin;
			if(s2->xout<minx) minx=s2->xout;
			if(s1->xin>maxx) maxx=s1->xin;
			if(s1->xout>maxx) maxx=s1->xout;

			take=ping ? runs2 : runs1;
			put=ping ? runs1 : runs2;
			ping=!ping;
			mergespans(spantab, put, take, minx, maxx, v);
		}
		take=ping ? runs2 : runs1;
		minx=0;
		while((j=*take))
		{
			if(take[1] & fill_rule)
				renderfunc(c, minx, y, j);
			minx+=j;
			take+=2;
		}
		
	}
}




void lineargradient(SDL_svg_context *c, int x, int y, int w)
{
float cx,cy;
float vx,vy;
float f;
float dx,dy,dyvy;
int dp;
int policy;

	policy = c->gradient_policy;


	cx=c->gradient_p1.x;
	cy=c->gradient_p1.y;

	vx=c->gradient_p2.x - cx;
	vy=c->gradient_p2.y - cy;

	f=vx*vx+vy*vy;

	dy=y-cy;
	dyvy = dy*vy;

	while(w-- > 0)
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
		colordot(c->surface, x++, y, c->gradient_colors[dp]);
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

void radialgradient(SDL_svg_context *c, int x, int y, int w)
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
	for(;w-->0;++x)
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
		colordot(c->surface, x, y, c->gradient_colors[dp]);
	}

}

void solidstrip(SDL_svg_context *c, int x, int y, int w)
{
	while(w-->0)
		colordot(c->surface, x++, y, c->solidcolor);
}

void solid(SDL_svg_context *c)
{
int i,j,k;
int colorstops;
void (*renderfunc)(SDL_svg_context *c, int x, int y, int w);
float minx, miny, maxx, maxy;
IPoint *ip;
int R,G,B,A;
IPoint *path;
int n;
int startnum;
struct spantab spans;

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

	R = (c->FillColor & 0x00ff0000) >> 16;
	G = (c->FillColor & 0x0000ff00) >> 8;
	B = (c->FillColor & 0x000000ff);
	A = 255; // temporary

	c->solidcolor=maprgb(c->surface, R,G,B) | (A<<24);

	renderfunc = solidstrip; // fall back on this...

	if(c->paint->type == SVG_PAINT_TYPE_GRADIENT)
	{
		colorstops = c->paint->p.gradient->num_stops;
		for(i=0;i<colorstops-1;++i)
		{
			int c1,c2;
			int r1,g1,b1,a1,r2,g2,b2,a2;
			unsigned long t;
			int v;

			c1=NUM_GRADIENT_COLORS*i/(colorstops-1);
			c2=NUM_GRADIENT_COLORS*(i+1)/(colorstops-1);

			t=c->paint->p.gradient->stops[i].color.rgb;
			a1=255.0 * c->paint->p.gradient->stops[i].opacity;
			r1 = (t>>16) & 255;
			g1 = (t>>8) & 255;
			b1 = t & 255;

			t=c->paint->p.gradient->stops[i+1].color.rgb;
			a2=255.0 * c->paint->p.gradient->stops[i+1].opacity;
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

		c->gradient_policy = c->paint->p.gradient->spread;
		if (c->paint->p.gradient->type == SVG_GRADIENT_LINEAR)
		{
			renderfunc = lineargradient;
			c->gradient_p1 = (IPoint)
				{ c->paint->p.gradient->u.linear.x1.value,
				  c->paint->p.gradient->u.linear.y1.value};
			c->gradient_p2 = (IPoint)
				{ c->paint->p.gradient->u.linear.x2.value,
				  c->paint->p.gradient->u.linear.y2.value};
		} else if(c->paint->p.gradient->type == SVG_GRADIENT_RADIAL)
		{
			renderfunc = radialgradient;
			c->gradient_p1 = (IPoint)
				{ c->paint->p.gradient->u.radial.cx.value,
				  c->paint->p.gradient->u.radial.cy.value};
			c->gradient_p2 = (IPoint)
				{ c->paint->p.gradient->u.radial.fx.value,
				  c->paint->p.gradient->u.radial.fy.value};
			c->gradient_r = c->paint->p.gradient->u.radial.r.value;
		}

		if(c->paint->p.gradient->units==SVG_GRADIENT_UNITS_USER)
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

	}

	initspantab(&spans);

	for(k=startnum=0;k<c->numpathstops;++k)
	{
		n=c->pathstops[k];

		generatespans(&spans, c, path+startnum, n-startnum);
		startnum = n;

	}
	renderspans(&spans, renderfunc, c);
}


void trace(SDL_Surface *thescreen, IPoint *path, int n,
		int r, int g, int b, int a)
{
int i,j;
unsigned long color;

	color=maprgb(thescreen, r,g,b);
	for(i=0;i<n;++i)
	{
		j=(i+1);
		if(j==n) j=0;
		vector(colordot, thescreen, (int)path[i].x, (int)path[i].y,
			(int)path[j].x, (int)path[j].y, color);
	}
}

