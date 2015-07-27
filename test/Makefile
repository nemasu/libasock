
all: test_simple_send_recv BufferQueueTest

test_simple_send_recv: simple_send_recv.cpp
	g++ -g -std=c++11 -L../ -I../ simple_send_recv.cpp ../libasock.so -o test_simple_send_recv

test_recv_spitter: recv_spitter.cpp
	g++ -g -std=c++11 -L../ -I../ recv_spitter.cpp ../libasock.so -o test_recv_spitter

BufferQueueTest: BufferQueueTest.cpp
	g++ -g -std=c++11 -L../ -I../ BufferQueueTest.cpp ../libasock.so -o test_BufferQueueTest
	
clean:
	rm -rf *.o test_*
