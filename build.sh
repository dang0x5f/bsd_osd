#!/bin/sh

PROG="osd"

CFLAGS="-ggdb -Wall -Wextra -pedantic"

FREETYPE2_FLAGS=`pkg-config --cflags freetype2`
FREETYPE2_LIBS=`pkg-config --libs freetype2`
XFT_LIBS=`pkg-config --libs xft`

cmd="cc ${CFLAGS} -o ${PROG} ${FREETYPE2_FLAGS} -I/usr/local/include -L/usr/local/lib ${XFT_LIBS} ${FREETYPE2_LIBS} -lX11 -lmixer osd.c"

echo ${cmd}
${cmd}
