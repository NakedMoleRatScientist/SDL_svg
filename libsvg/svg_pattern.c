/* svg_pattern.c: Data structures for SVG pattern elements
 
   Copyright © 2002 USC/Information Sciences Institute
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Steven Kramer
*/

#include "svgint.h"

#include <string.h>

svg_status_t
_svg_pattern_init (svg_pattern_t *pattern, svg_element_t *parent, svg_t *doc)
{
    svg_status_t status;

    _svg_length_init_unit (&pattern->x,
			   0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&pattern->y,
			   0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&pattern->width,
			   0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&pattern->height,
			   0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_HORIZONTAL);

    status = _svg_element_create (&pattern->group_element,
				  SVG_ELEMENT_TYPE_GROUP,
				  parent, doc);
    if (status)
	return status;
    
    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_pattern_init_copy (svg_pattern_t *pattern,
			svg_pattern_t *other)
{
    *pattern = *other;
    _svg_element_init_copy (pattern->group_element, other->group_element);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_pattern_deinit (svg_pattern_t *pattern)
{
    return _svg_element_deinit (pattern->group_element);
}

svg_status_t
_svg_pattern_apply_attributes (svg_pattern_t	*pattern,
			       const xmlChar	**attributes)
{
    int i;
    svg_transform_t transform;
    xmlChar const* str;
    
    _svg_attribute_get_string (attributes, "patternUnits", &str, "objectBoundingBox");
    if (strcmp (str, "userSpaceOnUse") == 0) {
	pattern->units = SVG_PATTERN_UNITS_USER;
    } else if (strcmp (str, "objectBoundingBox") == 0) {
	pattern->units = SVG_PATTERN_UNITS_BBOX;
    } else {
	return SVG_STATUS_INVALID_VALUE;
    }
    
    _svg_attribute_get_string (attributes, "patternContentUnits", &str, "userSpaceOnUse");

    if (strcmp (str, "userSpaceOnUse") == 0) {
	pattern->content_units = SVG_PATTERN_UNITS_USER;
    } else if (strcmp (str, "objectBoundingBox") == 0) {
	pattern->content_units = SVG_PATTERN_UNITS_BBOX;
    } else {
	return SVG_STATUS_INVALID_VALUE;
    }
    
    _svg_attribute_get_length (attributes, "x", &pattern->x, "0");
    _svg_attribute_get_length (attributes, "y", &pattern->y, "0");
    _svg_attribute_get_length (attributes, "width", &pattern->width, "0");
    _svg_attribute_get_length (attributes, "height", &pattern->height, "0");
    _svg_transform_init (&transform);
    _svg_attribute_get_string (attributes, "patternTransform", &str, 0);

    if (str) {
	_svg_transform_parse_str (&transform, str);
    }

    for (i = 0 ; i < 6 ; i++) {
	pattern->transform [i] = transform.m[i/2][i%2];
    }
    
    return SVG_STATUS_SUCCESS;
}
