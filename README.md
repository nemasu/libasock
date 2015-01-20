# libasock
Asynchronous TCP socket library for Linux. 

This library is not ready for general use yet, I pulled it out of another project of mine, so it's set up a bit weird at the moment.

It uses non-blocking edge triggered sockets with epoll.

There are 2 threads, one for receiving and one for sending.

To achieve async behaivour, there are also 2 buffers, one for sending and one for recieving.

For a very simple example on how it would be used, check simple_send_recv.cpp in the test directory.

