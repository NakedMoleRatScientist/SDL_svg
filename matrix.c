#include <stdlib.h>
#include <math.h>
#include "SDL_svg.h"
#include "internals.h"


/*
   SDL_matrix.c

*/



/*
   Matrix looks like this:
   a   c   e
   b   d   f
   0   0   1
*/



void svg_matrix_init (svg_matrix_t *dst,
				float a, float b,
				float c, float d,
				float e, float f)
{
	dst->a = a;
	dst->b = b;
	dst->c = c;
	dst->d = d;
	dst->e = e;
	dst->f = f;
}

void svg_matrix_description (svg_matrix_t *m)
{
	printf ("svg_matrix_t (%f %f %f %f  %f %f)\n",
			m->a, m->b, m->c, m->d, m->e, m->f);
}

void svg_matrix_translate(svg_matrix_t *m, float dx, float dy)
{
	m->e+=dx;
	m->f+=dy;
}

void svg_matrix_scale(svg_matrix_t *m, float sx, float sy)
{
	m->a*=sx;
	m->b*=sx;
	m->c*=sy;
	m->d*=sy;
}

IPoint svg_apply_matrix(svg_matrix_t *m, IPoint p)
{
IPoint n;
	n.x = m->a * p.x + m->c * p.y + m->e;
	n.y = m->b * p.x + m->d * p.y + m->f;
	return n;
}

IPoint svg_apply_matrix_without_translation(svg_matrix_t *m, IPoint p)
{
IPoint n;
	n.x = m->a * p.x + m->c * p.y;
	n.y = m->b * p.x + m->d * p.y;
	return n;
}

void svg_matrix_multiply(svg_matrix_t *dest,
		svg_matrix_t *left, svg_matrix_t *right)
{
svg_matrix_t res;

	res.a = left->a*right->a + left->c*right->b;
	res.c = left->a*right->c + left->c*right->d;
	res.e = left->a*right->e + left->c*right->f + left->e;

	res.b = left->b*right->a + left->d*right->b;
	res.d = left->b*right->c + left->d*right->d;
	res.f = left->b*right->e + left->d*right->f + left->f;
	*dest = res;
}

svg_matrix_t svg_matrix_invert(svg_matrix_t *in)
{
float det;
svg_matrix_t dst;

	det = in->a * in->d - in->b * in->c;

	if(det == 0.0)
		return (svg_matrix_t) {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
	dst.a = in->d/det;
	dst.b = -in->b/det;
	dst.c = -in->c/det;
	dst.d = in->a/det;
	dst.e = (in->c * in->f - in->d * in->e)/det;
	dst.f = (in->b * in->e - in->a * in->f)/det;
	return dst;

}
