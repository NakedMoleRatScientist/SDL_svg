# Makefile.am for the SDL svg renderer

lib_LTLIBRARIES = libSDL_svg.la

libSDL_svgincludedir = $(includedir)/SDL
libSDL_svginclude_HEADERS =	\
	SDL_svg.h

libSDL_svg_la_SOURCES =		\
	SDL_svg.c \
	render.c \
	matrix.c \
	ftgrays.c \
	libsvg/svg.c            libsvg/svg_group.c   libsvg/svg_pattern.c \
	libsvg/svg_ascii.c      libsvg/svg_str.c \
	libsvg/svg_attribute.c  libsvg/svg_length.c  libsvg/svg_style.c \
	libsvg/svg_color.c      libsvg/svg_paint.c   libsvg/svg_text.c \
	libsvg/svg_element.c    libsvg/svg_parser.c  libsvg/svg_transform.c \
	libsvg/svg_gradient.c   libsvg/svg_path.c

EXTRA_DIST =			\
	CHANGES			\
	SDL_svg.spec		\
	SDL_svg.spec.in		\
	autogen.sh

libSDL_svg_la_LDFLAGS = 		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
libSDL_svg_la_LIBADD = $(XML_LIBS)
libSDL_svg_la_CFLAGS = $(XML_CFLAGS)

noinst_PROGRAMS = svgtest

svgtest_LDADD = libSDL_svg.la  $(XML_LIBS)

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: distcheck

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

