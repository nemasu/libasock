#include <iostream>
#include <AsyncTransport.h>

using namespace std;

class PacketImpl : public Packet {
	public:
		char data1;
		char data2;

		~PacketImpl() {}
};

class PacketParserImpl : public PacketParser {
	public:
		Packet* deserialize ( unsigned char *buffer, unsigned int bufferSize, unsigned int *bufferUsed ) {
			if( bufferSize >= 2 ) {
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
			out[0] = dynamic_cast<PacketImpl*>(pkt)->data1;
			out[1] = dynamic_cast<PacketImpl*>(pkt)->data2;
			return out;
		}

		~PacketParserImpl() {}
};

int
main( int argv, char **argc ) {
	
	PacketParserImpl packetParser; 
	AsyncTransport asyncTransport( *((PacketParser *) &packetParser) );

	asyncTransport.init( 7290 );
	asyncTransport.start();

	AsyncTransport clientTransport( *((PacketParser *) &packetParser) );
	clientTransport.init( "localhost", 7290 );
	clientTransport.start();

	PacketImpl *packet = new PacketImpl();
	packet->data1 = 'h';
	packet->data2 = 'i';

	clientTransport.sendPacket((Packet*) packet);

	
	packet = (PacketImpl*) asyncTransport.getPacket();

	//First packet connet packet
	if (packet->type != PacketType::CONNECT) {
		cout << "FAIL" << endl;
		cout << "Failed to recieve CONNECT packet" << endl;
		return -1;
	}

	//Second packet data
	packet = (PacketImpl*) asyncTransport.getPacket();

	cout << "simple_send_recv: ";
	if (packet->data1 == 'h' && packet->data2 == 'i') {
		cout << "PASS" << endl;
	} else {
		cout << "FAIL" << endl;
		cout << "contents are:" << endl;
		cout << "data1: " << packet->data1 << endl;
		cout << "data2: " << packet->data2 << endl;
		return -1;
	}

	clientTransport.closeFd(packet->fd);
	
	delete packet;
	
	return 0;
}
