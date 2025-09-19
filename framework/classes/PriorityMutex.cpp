# include <PriorityMutex.hpp>

PriorityMutex::PriorityMutex() {}
PriorityMutex::~PriorityMutex() {}

void	PriorityMutex::lock(const Priority &priority) {
	if (priority == Priority::HIGHT) {
		_nextToAcces.lock();
		_dataMutex.lock();
		_nextToAcces.unlock();
	}
	else if (priority == Priority::LOW) {
		_lowPriorityAccess.lock();
		_nextToAcces.lock();
		_dataMutex.lock();
		_nextToAcces.unlock();
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
