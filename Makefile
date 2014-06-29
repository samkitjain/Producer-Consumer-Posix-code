# 	Name: Samkit Jain

all: clean prodcon

prodcon: main.c prodcon.c prodcon.h
	$(CC) -D_REENTRANT main.c -o main -lpthread
	$(CC) -D_REENTRANT prodcon.c -o prodcon -lpthread

clean:
	rm -rf main.o prodcon.o
	
