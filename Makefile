CC = gcc
CFLAGS = -std=c11 -O3 -march=native -ffast-math -funroll-loops -Wall -Wextra
OBJS = memstack.o matrix.o main.o

all: matdemo

memstack.o: memstack.c memstack.h types.h
	$(CC) $(CFLAGS) -c memstack.c

matrix.o: matrix.c matrix.h memstack.h types.h
	$(CC) $(CFLAGS) -c matrix.c

main.o: main.c matrix.h memstack.h types.h
	$(CC) $(CFLAGS) -c main.c

matdemo: $(OBJS)
	$(CC) $(CFLAGS) -o matdemo $(OBJS)

clean:
	rm -f *.o matdemo