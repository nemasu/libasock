# libasock
Asynchronous C++ TCP socket library for Linux. 

It uses non-blocking edge triggered sockets with epoll.

There are 2 threads, one for receiving and one for sending.

To achieve async behaivour, there are also 2 buffers, one for sending and one for recieving.

For a very simple example on how it would be used, check simple_send_recv.cpp in the test directory.

