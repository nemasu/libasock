#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <mutex>
#include <condition_variable>

using std::condition_variable;
using std::mutex;

class Trigger {
	public:
		Trigger() {
			sig = 0;
		}
		
		void notify();
		void wait();

	private:
		condition_variable condVar;	
		mutex waitMutex;
		int sig;
};
#endif
