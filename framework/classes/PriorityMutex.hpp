# pragma once

/// System includes
# include <mutex>

enum class Priority {
	HIGHT,
	LOW
};

class PriorityMutex {
	private:
		std::mutex	_lowPriorityAccess;
		std::mutex	_nextToAccess;
		std::mutex	_dataMutex;
	
	public:
		PriorityMutex();
		~PriorityMutex();

		void	lock(const Priority &priority);
		void	unlock(const Priority &priority);
};
