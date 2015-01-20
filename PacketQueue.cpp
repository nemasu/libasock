#include "PacketQueue.h"

using std::lock_guard;

void
PacketQueue::wait() {
	packetTrigger.wait();
}

void
PacketQueue::push(Packet * pkt) {
	lock_guard<mutex> lock( qMutex );
	q.push( pkt );
	packetTrigger.notify();
}

Packet*
PacketQueue::pop() {
	lock_guard<mutex> lock( qMutex );
	Packet *ret = NULL;	
	if( q.size() > 0 ) {
		ret = q.front();
		q.pop();
	}
	return ret;
}
