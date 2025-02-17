# pragma once

/// System includes
# include <stdint.h>
# include <chrono>
# include <string>
# include <map>
# include <float.h>

/// Global variables

typedef struct timeLog {
	timeLog() : max(0), av(0), min(FLT_MAX) {};

	float	max;
	float	av;
	float	min;
}	timeLog;

// The Profiler class allow to evaluate performance in specific context of the program.
// It can:
//  - Benchmark (void) and non (void) returning function.
//  - Print wanted data to the standard output or in a log file output.
class	Profiler {
	private:
		std::map<std::string, timeLog>	_logs;

	public:
		Profiler();
		~Profiler();

		void	printLog();
		void	printLog(const std::string &funcName);
		void	logToFile(const std::string &fileName);

		// That method takes in a function and run it normally will monitoring it's execution speed.
		// It will return the output expected by the function passed in parameters
		//  - 1st argument is the function identifier and will be outputed in the logs as such.
		//  - 2nd argument is the function pointer.
		//  - Any arguments from that point will be the 2nd argument parameters.
		//  | They must be put in the order expected by the second argument and their type must be explicite.
		//  | Consecutive parameters that have the same type must be in the same "coma space".
		//  |  - expample => foo(param1(string) param2(string), param3(int), param4(size_t));
		// ---------------------------------------------------------------------------------
		//
		// A valid utilization of that function would be:
		// | int	foo(int, int, char);
		// | int returnValue = profilerObject.evaluate("foo", &foo, 4 5, 'c');
		template <typename T, typename... U>
		T	evaluate(const std::string &funcName,T (func)(U...),
				U... arg1) {
			T	returnValue;

			auto	start = std::chrono::steady_clock::now();
			returnValue = func(arg1...);
			auto	end = std::chrono::steady_clock::now();
			unsigned int	nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			float	mill = (float)nano / 1000000;
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

		// That method is identical to Profiler::evaluate(); but returns nothing.
		// Use that method if the function passed in argument does not return anything.
		// Please refere to it for more details.
		template <typename T, typename... U>
		void	evaluateNoReturn(const std::string &funcName,T (func)(U...), U... arg) {
			auto	start = std::chrono::steady_clock::now();
			func(arg...);
			auto	end = std::chrono::steady_clock::now();
			unsigned int	nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			float	mill = (float)nano / 1000000;
			if (this->_logs[funcName].min > mill)
				this->_logs[funcName].min = mill;
			if (this->_logs[funcName].max < mill)
				this->_logs[funcName].max = mill;
			if (this->_logs[funcName].av == 0)
				this->_logs[funcName].av = mill;
			else
				this->_logs[funcName].av = (this->_logs[funcName].av + mill) / 2.0f;
		}
		template <typename T>
		void	evaluateNoReturn(const std::string &funcName,T (func)()) {
			auto	start = std::chrono::steady_clock::now();
			func();
			auto	end = std::chrono::steady_clock::now();
			unsigned int	nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			float	mill = (float)nano / 1000000;
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
