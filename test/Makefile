
all: test_simple_send_recv test_recv_spitter

test_simple_send_recv: simple_send_recv.cpp
	g++ -g -std=c++11 -L../ -I../ simple_send_recv.cpp ../libasock.so -o test_simple_send_recv

test_recv_spitter: recv_spitter.cpp
	g++ -g -std=c++11 -L../ -I../ recv_spitter.cpp ../libasock.so -o test_recv_spitter

clean:
	rm -rf *.o test_*
