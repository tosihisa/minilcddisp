
CC := gcc
TARGET := minilcddisp
CFLAGS := -O2 -Wall -Wextra -fdiagnostics-show-option -I./linuxfont

all: minilcddisp

${TARGET}: minilcddisp.c \
	linuxfont/font_10x18.c \
	linuxfont/font_6x11.c \
	linuxfont/font_7x14.c \
	linuxfont/font_8x16.c \
	linuxfont/font_8x8.c \
	linuxfont/font_acorn_8x8.c \
	linuxfont/font_mini_4x6.c \
	linuxfont/font_pearl_8x8.c \
	linuxfont/font_sun12x22.c \
	linuxfont/font_sun8x16.c \
	Oled704_1306_13SU.c
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f ${TARGET} *.o

