CC=g++

CFLAGS=-std=c++11 -pthread

all: alice.o bob.o carol.o

alice.o: alice.cpp 
	$(CC) $(CFLAGS) alice.cpp -o alice.o

bob.o: bob.cpp
	$(CC) $(CFLAGS) bob.cpp -o bob.o

carol.o: carol.cpp
	$(CC) $(CFLAGS) carol.cpp -o carol.o

clean:
	rm *.o

