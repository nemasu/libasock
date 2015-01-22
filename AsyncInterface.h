#ifndef __ASYNCINTERFACE_H__
#define __ASYNCINTERFACE_H__

#include <string>
#include <map>

enum PacketType {
	NORMAL,
	DISCONNECT
};

//Inherit from Packet for your packets.
class Packet {
	public:	

		Packet(PacketType t = NORMAL) {
			type = t;
		}

		int fd;
		PacketType type;
		
		void
		setOrigin(Packet *pkt) {
			fd = pkt->fd;
		}
	};

//Implement this interface.
class PacketParser {
	public:
		//Tries to deserialize buffer, returns NULL if unsuccessful, -1 if fatal error.
		//Buffer amount used stored in bufferUsed.
		//Packets are deleted in AsyncTransport::sendPacket
		virtual Packet*      deserialize ( unsigned char *buffer,  unsigned int bufferSize, unsigned int *bufferUsed ) = 0;

		//Returns serialization of pkt, size is written into out_size.
		virtual char*        serialize   ( Packet         *pkt,    unsigned int *out_size  ) = 0;

		virtual ~PacketParser(){};
};

#endif
