# include <PriorityMutex.hpp>

PriorityMutex::PriorityMutex() {}
PriorityMutex::~PriorityMutex() {}

void	PriorityMutex::lock(const Priority &priority) {
	if (priority == Priority::HIGHT) {
		_nextToAccess.lock();
		_dataMutex.lock();
		_nextToAccess.unlock();
	}
	else if (priority == Priority::LOW) {
		_lowPriorityAccess.lock();
		_nextToAccess.lock();
		_dataMutex.lock();
		_nextToAccess.unlock();
	}
}

void	PriorityMutex::unlock(const Priority &priority) {
	if (priority == Priority::HIGHT)
		_dataMutex.unlock();
	else if (priority == Priority::LOW) {
		_dataMutex.unlock();
		_lowPriorityAccess.unlock();
	}
}
