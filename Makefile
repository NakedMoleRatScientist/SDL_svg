
GCC = gcc
TARGET = obj
VPATH = ..
CFLAGS=-Wall -O2
CFLAGS += -I/usr/local/include/SDL
CFLAGS += -I../libsvg
#CFLAGS += -DVERBOSE
#CFLAGS += -g

LDFLAGS += -L/usr/X11R6/lib

SRC = matrix.c render.c SDL_svg.c
OBJ = $(SRC:.c=.o)
LIB = libSDL_svg.so
DEPS = SDL_svg.h Makefile

all:
	mkdir -p $(TARGET)
	make -C $(TARGET) -f ../Makefile $(LIB) svgtest

$(LIB):  $(OBJ)
	$(CC) -shared -o $(LIB) $(OBJ)

svgtest: svgtest.o $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS) -lm -lsvg -lpng -ljpeg -lxml2 -lSDL \
		-lpthread -lX11 -lXext

svgtest.o: svgtest.c $(DEPS)
matrix.o: matrix.c $(DEPS)
SDL_svg.o: SDL_svg.c $(DEPS)
render.o: render.c $(DEPS)

install:
	cp $(TARGET)/$(LIB) /usr/local/lib

clean:
	rm -rf $(TARGET)
