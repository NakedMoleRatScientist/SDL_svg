
TARGET = obj
GCC = gcc
CFLAGS = -O2 -Wall
CFLAGS += -I/usr/local/include/libxml2
VPATH = ..
LIB = libsvg.so

OBJS = 	svg.o svg_ascii.o svg_attribute.o svg_color.o \
	svg_element.o svg_gradient.o svg_group.o svg_length.o \
	svg_paint.o svg_parser.o svg_pattern.o svg_path.o \
	svg_str.o svg_style.o svg_text.o svg_transform.o

#uncomment these lines below for the image capability
#OBJS += svg_image.c
#CFLAGS += -DIMAGE_CRAP

HEADERS = svg.h svgint.h svg_ascii.h svg_version.h config.h

all:
	mkdir -p $(TARGET)
	make -C $(TARGET) -f ../Makefile $(LIB)

install:
	cp $(TARGET)/libsvg.so /usr/local/lib

clean:
	rm -rf $(TARGET)

$(LIB): $(OBJS)
	$(GCC) -o $(LIB) -shared $(OBJS)

.c.o:
	$(GCC) -c -o $@ $(CFLAGS) $<

svg.o:			svg.c $(HEADERS)
svg_ascii.o:		svg_ascii.c $(HEADERS)
svg_image.o:		svg_image.c $(HEADERS)
svg_attribute.o:	svg_attribute.c $(HEADERS)
svg_color.o:		svg_color.c $(HEADERS)
svg_element.o:		svg_element.c $(HEADERS)
svg_gradient.o:		svg_gradient.c $(HEADERS)
svg_group.o:		svg_group.c $(HEADERS)
svg_length.o:		svg_length.c $(HEADERS)
svg_paint.o:		svg_paint.c $(HEADERS)
svg_parser.o:		svg_parser.c $(HEADERS)
svg_pattern.o:		svg_pattern.c $(HEADERS)
svg_path.o:		svg_path.c $(HEADERS)
svg_str.o:		svg_str.c $(HEADERS)
svg_style.o:		svg_style.c $(HEADERS)
svg_text.o:		svg_text.c $(HEADERS)
svg_transform.o:	svg_transform.c $(HEADERS)
