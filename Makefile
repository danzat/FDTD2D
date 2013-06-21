CFLAGS=-Wall -Wno-unused-variable -pedantic -g -std=gnu99 -I/usr/X11R6/include `pkg-config --cflags cairo theoraenc`
LDFLAGS=-Wall -Wno-unused-variable -g `pkg-config --libs cairo theoraenc` -L/usr/X11R6/lib -lX11

all: main

clean:
	rm *.o

main: main.o fdtd2d.o excitation.o video.o assignment.o
	gcc -o main main.o fdtd2d.o excitation.o video.o assignment.o ${LDFLAGS}

main.o: main.c
	gcc -o main.o ${CFLAGS} -c main.c

fdtd2d.o: fdtd2d.c fdtd2d.h
	gcc -o fdtd2d.o ${CFLAGS} -c fdtd2d.c

assignment.o: assignment.c assignment.h
	gcc -o assignment.o ${CFLAGS} -c assignment.c

excitation.o: excitation.c excitation.h

video.o: video.c video.h
	gcc -o video.o ${CFLAGS} -c video.c
