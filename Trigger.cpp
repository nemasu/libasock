#include "Trigger.h"

using std::unique_lock;
using std::lock_guard;

void
Trigger::wait() {
    unique_lock<mutex> lk(waitMutex);
    if( !sig ) {
        condVar.wait( lk );
    }
	sig--;
}

void
Trigger::notify() {
	lock_guard<mutex> lk(waitMutex);
	sig++;
	condVar.notify_one();
}
	

