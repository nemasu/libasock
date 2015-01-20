
all: test

test: main.cpp
	g++ -g -std=c++11 -L../ -I../ main.cpp ../libasock.so -o main

clean:
	rm -rf *.o main
