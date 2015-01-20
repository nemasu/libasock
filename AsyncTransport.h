#include "AsyncInterface.h"
#include "BufferQueue.h"
#include "PacketQueue.h"

#ifndef __ASYNCSOCKET_H__
#define __ASYNCSOCKET_H__

//TODO This needed?
#define MAX_PACKET_SIZE 1024

using std::string;

struct ConnectionData {
    unsigned int  fd;
    unsigned int  bufferSize;
    unsigned char buffer[MAX_PACKET_SIZE];
};

class AsyncTransport {
	public:
		AsyncTransport( PacketParser * );
		~AsyncTransport(){}

		bool init( int port );
		bool init( string serverHost, int port );
		void start();
	
		void sendPacket( Packet * pkt );
		Packet* getPacket();

		void closeFd( int fd );
	private:
		PacketParser *packetParser;
		BufferQueue  *bufferQueue;
		PacketQueue  *packetQueue;
		static void receiveData( AsyncTransport * );
		static void sendData( AsyncTransport * );
		int listenFD;
		int epollSendFD;
		mutex closeMutex;
		bool isServer;
};
#endif
