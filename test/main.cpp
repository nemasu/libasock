#include <iostream>
#include <AsyncTransport.h>

using namespace std;

class PacketImpl : public Packet {
	public:
		char data1;
		char data2;
};

class PacketParserImpl : public PacketParser {
	Packet* deserialize ( unsigned char *buffer, unsigned int bufferSize, unsigned int *bufferUsed ) {
		if( bufferSize > 2 ) {
			PacketImpl* packet = new PacketImpl();
			packet->data1 = buffer[0];
			packet->data2 = buffer[1];
			(*bufferUsed) = 2;
			return (Packet*) packet;
		} else {
			return NULL;
		}
	}

	char * serialize ( Packet *pkt, unsigned int *out_size ) {
		char *out = new char[2];
		(*out_size) = 2;
		out[0] = reinterpret_cast<PacketImpl*>(pkt)->data1;
		out[1] = reinterpret_cast<PacketImpl*>(pkt)->data2;
		return out;
	}
};

int
main( int argv, char **argc ) {
	
	PacketParser *packetParser = new PacketParserImpl();
	AsyncTransport asyncTransport( packetParser );

	asyncTransport.init( 7290 );
	asyncTransport.start();

	while(1) {
		Packet *packet = asyncTransport.getPacket();
		cout << "Got a packet!" << endl;
	}

	return 0;
}
