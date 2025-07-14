.POSIX:

CC := clang

CFLAGS := -std=c23 -Wall -Wextra -pedantic
# CFLAGS := -std=c23 -ggdb -Wall -Wextra -pedantic

X11CPP := `pkg-config --cflags x11`
XFTCPP := `pkg-config --cflags xft`
CPPFLAGS := ${X11CPP} ${XFTCPP}

X11LD := `pkg-config --libs x11`
XFTLD := `pkg-config --libs xft`
FCLD  := `pkg-config --libs fontconfig`
LDFLAGS := ${X11LD} ${XFTLD} ${FCLD} -lmixer

BIN := osd
BINDIR := ${HOME}/bin/

SRCS := osd.c
OBJS := ${SRCS:c=o}

${BIN}: ${OBJS}
	${CC} -o $@ ${LDFLAGS} ${OBJS}

install: ${OBJS}
	${CC} -o ${BIN} ${LDFLAGS} ${OBJS}
	mv ${BIN} ${BINDIR}${BIN}

${OBJS}: ${SRCS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c $<

clean:
	rm -f ${BIN} *.o *.d *.core

.PHONY: clean
