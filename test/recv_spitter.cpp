#include <iostream>
#include <AsyncTransport.h>
#include <string.h>

using namespace std;

class PacketImpl : public Packet {
	public:
		char *data;
		unsigned int size;
};

class PacketParserImpl : public PacketParser {
	Packet* deserialize ( unsigned char *buffer, unsigned int bufferSize, unsigned int *bufferUsed ) {		
		cout << "Got buffer of size: " << bufferSize << endl;
		
		PacketImpl* packet = new PacketImpl();
	
		packet->data = new char[bufferSize];
		packet->size = bufferSize;
		
		memcpy( packet->data, buffer, bufferSize );
		(*bufferUsed) = bufferSize;
		
		return (Packet*) packet;
	}

	char * serialize ( Packet *pkt, unsigned int *out_size ) {
		return NULL;
	}
};

int
main( int argv, char **argc ) {
	
	PacketParser *packetParser = new PacketParserImpl();
	AsyncTransport asyncTransport( packetParser );

	asyncTransport.init( 80 );
	asyncTransport.start();

	while(1) {
		PacketImpl *packet = (PacketImpl*) asyncTransport.getPacket();

		if (packet->type == PacketType::NORMAL) {
			for (int i = 0; i < packet->size; i++) {
				cout << packet->data[i];
			}

			cout << endl;
		}

		delete packet;
	}

	return 0;
}
