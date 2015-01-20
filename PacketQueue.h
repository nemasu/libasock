#ifndef __PACKETQUEUE_H__
#define __PACKETQUEUE_H__

#include "AsyncInterface.h"
#include "Trigger.h"
#include <mutex>
#include <queue>

using std::queue;

class PacketQueue {
	public:
		PacketQueue(){}
		~PacketQueue(){}

		void push(Packet *);
		Packet* pop();
		void wait();
	
	private:
		queue<Packet*> q;
		mutex qMutex;
		Trigger packetTrigger;
};
#endif
