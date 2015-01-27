#include "Trigger.h"

using std::unique_lock;
using std::lock_guard;

void
Trigger::wait() {
    unique_lock<mutex> lk(waitMutex);
    while( sig == 0 ) {
        condVar.wait( lk );
    }
	sig--;
}

void
Trigger::notify() {
	unique_lock<mutex> lk(waitMutex);
	sig++;
	condVar.notify_one();
}
	

