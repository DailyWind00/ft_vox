# pragma once

/// Define

/// System includes
# include <stdint.h>
# include <chrono>
# include <string>
# include <map>
# include <float.h>

/// Dependencies

typedef struct timeLog {
	timeLog() : max(0), av(0), min(FLT_MAX) {};

	float	max;
	float	av;
	float	min;
}	timeLog;

# include <unistd.h>
class	Profiler {
	private:
		std::map<std::string, timeLog>	_logs;

	public:
		Profiler();
		~Profiler();

		void	printLog();
		void	printLog(const std::string &funcName);
		void	logToFile();

		template <typename T, typename... U>
		T	evaluate(const std::string &funcName,T (func)(U...),
				U... arg1) {
			T	returnValue;

			auto	start = std::chrono::steady_clock::now();
			returnValue = func(arg1...);
			auto	end = std::chrono::steady_clock::now();
			unsigned int	nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			float	mill = (float)nano / 1000;
			if (this->_logs[funcName].min > mill)
				this->_logs[funcName].min = mill;
			if (this->_logs[funcName].max < mill)
				this->_logs[funcName].max = mill;
			if (this->_logs[funcName].av == 0)
				this->_logs[funcName].av = mill;
			else
				this->_logs[funcName].av = (this->_logs[funcName].av + mill) / 2.0f;
			return (returnValue);
		}

		template <typename T, typename... U>
		void	evaluateNoReturn(const std::string &funcName,T (func)(U...), U... arg) {
			auto	start = std::chrono::steady_clock::now();
			func(&arg...);
			auto	end = std::chrono::steady_clock::now();
			unsigned int	nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			float	mill = (float)nano / 1000;
			if (this->_logs[funcName].min > mill)
				this->_logs[funcName].min = mill;
			if (this->_logs[funcName].max < mill)
				this->_logs[funcName].max = mill;
			if (this->_logs[funcName].av == 0)
				this->_logs[funcName].av = mill;
			else
				this->_logs[funcName].av = (this->_logs[funcName].av + mill) / 2.0f;
		}
};
