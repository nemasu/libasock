#ifndef __ASYNCSOCKET_H__
#define __ASYNCSOCKET_H__

#include "AsyncInterface.h"
#include "PacketQueue.h"
#include "BufferQueue.h"
#include <mutex>
#include <string>
#include <set>

//TODO This is huge. 65KB+ per connection too much overhead?
//Or, smaller buffer, then copy into resizing ConnectionData buffer...
#define MAX_PACKET_SIZE 65535

using std::string;
using std::mutex;
using std::set;

struct ConnectionData {
    unsigned int  fd;
    unsigned int  bufferSize;
    unsigned char buffer[MAX_PACKET_SIZE];
};

class AsyncTransport {
	public:
		AsyncTransport( PacketParser & );
		~AsyncTransport();

		bool init( int port );
		bool init( string serverHost, int port );
		void start();
		void stop();
	
		void sendPacket( Packet * pkt );
		Packet* getPacket();

		void closeFd( int fd );
	private:
		PacketParser &packetParser;
		BufferQueue  bufferQueue;
		PacketQueue  packetQueue;
		set<ConnectionData*> pendingData;
		static void receiveData( AsyncTransport & );
		static void sendData( AsyncTransport & );
		int epollSendFD;
		mutex closeMutex;

		//fd is the server endpoint on client,
		//and the listen fd on server.
		int fd;
		bool isServer;
		volatile bool isRunning;
};
#endif
