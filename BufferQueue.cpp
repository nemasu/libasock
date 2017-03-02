#include "BufferQueue.h"
#include <errno.h>
#include <string.h>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

using std::cerr;
using std::endl;

void
BufferQueue::get( int fd, char *&out_buffer, int *out_length) {
    lock_guard<mutex> lock( qMutex );

    if( byteQueue.count( fd ) > 0 ) {
        char *buffer = byteQueue[fd];
        int length = qLength[fd];
        
        out_buffer = buffer;
        *out_length = length;
    } else {
        *out_length = 0;
    }
}


void
BufferQueue::put( int fd, char *buffer, int length) {
    lock_guard<mutex> lock( qMutex );

    char *curBuffer = NULL;
    int  curLength  = 0;

    if( byteQueue.count( fd ) > 0 ) {
        curBuffer = byteQueue[fd];
        curLength = qLength[fd];

        char *newBuffer = new char[length+curLength];
        memcpy( newBuffer, curBuffer, curLength );
        delete [] curBuffer;
        //TODO kill connection if send buffer hits a limit?

        curBuffer = newBuffer;

    } else {
        curBuffer = new char[length];
        curLength = 0;
    }
    
    
    memcpy( curBuffer+curLength, buffer, length );
    byteQueue[fd] = curBuffer;
    qLength[fd] = curLength + length;

    //epoll stuff
    epoll_event ev;

    ev.events = EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    if( epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &ev ) == -1 ) {
        cerr << "BufferQueue::put epoll_ctl ADD failed: " << strerror( errno ) << endl;   
    }
}

void
BufferQueue::updateUsed( int fd, int length ) {
    lock_guard<mutex> lock( qMutex );
    
    char *buf = byteQueue[fd];
    if( qLength[ fd ] == length ) {
        //All done! Remove from epoll
        if( epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, NULL ) == -1 ) {
            cerr << "BufferQueue::updateUsed epoll_ctl DEL failed." << strerror( errno ) << endl;
        }
        
        delete[] buf;
        byteQueue.erase(fd);
        qLength.erase(fd);
    } else {
        int new_length = qLength[ fd ] - length;
        char *newBuff = new char[new_length];
        memcpy( newBuff, buf+length, new_length );
        delete [] buf;
        byteQueue[fd] = newBuff;
        qLength[fd] = new_length;
    }
}
