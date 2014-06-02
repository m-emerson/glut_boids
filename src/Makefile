CC=gcc
CFLAGS=-framework OpenGL -framework GLUT -Wall -lSOIL -framework CoreFoundation
objects = boids.o

default: boids

boids.o: boids.c
	$(CC) -c boids.c -o boids.o $(CFLAGS)

boids: $(objects)
	$(CC) boids.o -o boids $(CFLAGS)

${objects}: boids.h
clean:
	-rm -f *.o
	-rm -f boids
