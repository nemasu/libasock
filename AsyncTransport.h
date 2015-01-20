#ifndef __ASYNCSOCKET_H__
#define __ASYNCSOCKET_H__

#include "AsyncInterface.h"
#include <mutex>
#include <string>

class BufferQueue;
class PacketQueue;


//TODO This needed?
#define MAX_PACKET_SIZE 1024

using std::string;
using std::mutex;

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
		int epollSendFD;
		mutex closeMutex;

		//fd is the server endpoint on client,
		//and the listen fd on server.
		int fd;
		bool isServer;
};
#endif
