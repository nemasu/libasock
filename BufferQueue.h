#ifndef __BUFFERQUEUE_H__
#define __BUFFERQUEUE_H__

#include "AsyncInterface.h"
#include <iostream>
#include <mutex>
#include <map>
#include <queue>

using std::map;
using std::mutex;
using std::lock_guard;

class BufferQueue {
	public:
		BufferQueue(){
		}

		void
		setEpollFd( int fd ) {
			epollFD = fd;
		}

		~BufferQueue(){}

		void get( int fd, char *&out_buffer, int *out_length);
		void put( int fd, char *buffer, int length);
		void updateUsed( int fd, int length );

		void closeFd( int fd ) {
			lock_guard<mutex> lock( qMutex );
			if( byteQueue.count( fd ) > 0 ) {
				delete[] byteQueue[fd];
				byteQueue.erase( fd );
				qLength.erase( fd );
			}
		}

	private:
		//fd, bytes
		map<int, char*> byteQueue;

		//fd, length
		map<int, int> qLength;

		map<int, bool> fdSending;

		mutex qMutex;

		int epollFD;
};
#endif
