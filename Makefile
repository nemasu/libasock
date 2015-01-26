CCOPTS=-std=c++11 -fpic -Wall -Werror

all: CCOPTS += -O2
all: libasock

debug: CCOPTS += -g
debug: libasock

libasock: BufferQueue.o AsyncTransport.o Trigger.o PacketQueue.o
	g++ ${CCOPTS} -shared -lpthread BufferQueue.o AsyncTransport.o Trigger.o PacketQueue.o -o libasock.so

BufferQueue.o: BufferQueue.cpp BufferQueue.h
	g++ -c ${CCOPTS} BufferQueue.cpp -o BufferQueue.o

AsyncTransport.o: AsyncTransport.cpp AsyncTransport.h
	g++ -c ${CCOPTS} AsyncTransport.cpp -o AsyncTransport.o

Trigger.o: Trigger.h Trigger.cpp
	g++ -c ${CCOPTS} Trigger.cpp -o Trigger.o

PacketQueue.o: PacketQueue.h PacketQueue.cpp
	g++ -c ${CCOPTS} PacketQueue.cpp -o PacketQueue.o

clean:
	rm -rf *.o libasock*

install: all
	mkdir -p /usr/include/libasock
	cp -v *.h /usr/include/libasock
	cp -v libasock.so /usr/lib

