#include <mutex>
#include <condition_variable>

#ifndef __TRIGGER_H__
#define __TRIGGER_H__

using std::condition_variable;
using std::mutex;

class Trigger {
	public:
		Trigger(){
			sig = 0;
		}
		~Trigger(){}

		void notify();
		void wait();

	private:
		condition_variable condVar;	
		mutex waitMutex;
		int sig;
};
#endif
