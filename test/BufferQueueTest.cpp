#include <string.h>
#include <iostream>
#include "../BufferQueue.h"

using namespace std;

extern "C"

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    return 0;
}

int
main( int argv, char **argc ) {

    char buffer[] = "hihihihi";
    int buffer_size = 8;

    int epollFd = 1;
    int fd = 2;

    BufferQueue bq;
    bq.setEpollFd(epollFd);
    bq.put(fd, buffer, buffer_size);

    char *buffer2 = nullptr;
    int out_length;

    bq.get( fd, buffer2, &out_length );

    cout << "BufferQueue put/get: ";
    if( !memcmp( buffer, buffer2, buffer_size ) ) {
        cout << "PASS" << endl;
    } else {
        cout << "FAIL" << endl;
    }

    bq.updateUsed( fd,  buffer_size/2 );

    bq.get( fd, buffer2, &out_length );
    cout << "BufferQueue updateUsed/get: ";
    if( !memcmp( buffer2, "hihi", 4 ) ) {
        cout << "PASS" << endl;
    } else {
        cout << "FAIL" << endl;
    }

    bq.updateUsed( fd, buffer_size/2 );
    
    bq.get( fd, buffer2, &out_length );
    cout << "BufferQueue updateUsed all: ";
    if( out_length == 0 ) {
        cout << "PASS" << endl;
    } else {
        cout << "FAIL" << endl;
        cout << "buffer: " << buffer2 << endl;
        cout << "out_length: " << out_length << endl;
    }

    return 0;
 }
