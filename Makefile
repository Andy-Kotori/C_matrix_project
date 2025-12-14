CC = gcc
CFLAGS = -std=c11 -O3 -march=native -ffast-math -funroll-loops -Wall -Wextra -IInc
OBJS = Src/memstack.o Src/matrix.o Src/main.o

all: matdemo

Src/memstack.o: Src/memstack.c Inc/memstack.h Inc/types.h
	$(CC) $(CFLAGS) -c Src/memstack.c -o Src/memstack.o

Src/matrix.o: Src/matrix.c Inc/matrix.h Inc/memstack.h Inc/types.h
	$(CC) $(CFLAGS) -c Src/matrix.c -o Src/matrix.o

Src/main.o: Src/main.c Inc/matrix.h Inc/memstack.h Inc/types.h
	$(CC) $(CFLAGS) -c Src/main.c -o Src/main.o

matdemo: $(OBJS)
	$(CC) $(CFLAGS) -o matdemo $(OBJS)

clean:
	rm -f Src/*.o matdemo