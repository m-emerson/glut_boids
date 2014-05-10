CC=gcc
CFLAGS=-framework OpenGL -framework GLUT

default: boids

boids.o: boids.c
	$(CC) -c boids.c -o boids.o $(CFLAGS)

boids: boids.o
	$(CC) boids.o -o boids $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f boids
