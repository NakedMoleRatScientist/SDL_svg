#include <stdlib.h>
#include <math.h>
#include "SDL_svg.h"
#include "internals.h"


/*
   SDL_matrix.c

*/



/*
   Matrix looks like this:
   A   C   E
   B   D   F
   0   0   1
*/



#define A matrix[0]
#define B matrix[1]
#define C matrix[2]
#define D matrix[3]
#define E matrix[4]
#define F matrix[5]

void svg_matrix_init (svg_matrix_t *dst,
				float a, float b,
				float c, float d,
				float e, float f)
{
	dst->A = a;
	dst->B = b;
	dst->C = c;
	dst->D = d;
	dst->E = e;
	dst->F = f;
}

void svg_matrix_description (svg_matrix_t *m)
{
	printf ("svg_matrix_t (%f %f %f %f  %f %f)\n",
			m->A, m->B, m->C, m->D, m->E, m->F);
}

IPoint svg_apply_matrix(svg_matrix_t *m, IPoint p)
{
IPoint n;
	n.x = m->A * p.x + m->C * p.y + m->E;
	n.y = m->B * p.x + m->D * p.y + m->F;
	return n;
}

IPoint svg_apply_matrix_without_translation(svg_matrix_t *m, IPoint p)
{
IPoint n;
	n.x = m->A * p.x + m->C * p.y;
	n.y = m->B * p.x + m->D * p.y;
	return n;
}

void svg_matrix_multiply(svg_matrix_t *dest,
		svg_matrix_t *left, svg_matrix_t *right)
{
svg_matrix_t res;

	res.A = left->A*right->A + left->C*right->B;
	res.C = left->A*right->C + left->C*right->D;
	res.E = left->A*right->E + left->C*right->F + left->E;

	res.B = left->B*right->A + left->D*right->B;
	res.D = left->B*right->C + left->D*right->D;
	res.F = left->B*right->E + left->D*right->F + left->F;
	*dest = res;
}
