/// Class independant system includes
# include <iostream>
# include <fstream>

# include "Profiler.hpp"


/// Constructors & Destructors
Profiler::Profiler() {
}

Profiler::~Profiler() {
}
/// ---



/// Public functions

// Output the all logs held by the profiler to the standard output
void	Profiler::printLog()
{
	if (!this->_logs.size())
		std::cout << "Profiler is not holding any data" << std::endl;

	for (auto a : this->_logs) {
		for (size_t i = 0; i < 15; i++)
			std::cout << "-";
		std::cout << "\n - " << a.first << "() => minimum: " << a.second.min << "ms"
							<< " | average: " << a.second.av << "ms"
						       	<< " | maximum: " << a.second.max << "ms" << std::endl;
	}
	for (size_t i = 0; i < 15; i++)
		std::cout << "-";
}

// Output the logs of fucName to the standard output
void	Profiler::printLog(const std::string &funcName)
{
	auto	a = this->_logs[funcName];

	std::cout << "\n - " << funcName << "() => minimum: " << a.min << "ms"
							<< " | average: " << a.av << "ms"
						       	<< " | maximum: " << a.max << "ms" << std::endl;
}

// Output the all logs held by the profiler to a file named "fileName.logs"
void	Profiler::logToFile(const std::string &fileName)
{
	if (!this->_logs.size())
		std::cout << "Profiler is not holding any data" << std::endl;

	std::ofstream	file(fileName + ".logs");

	if (!file.is_open())
		throw (std::runtime_error("could not output profiler logs"));

	for (auto a : this->_logs) {
		for (size_t i = 0; i < 15; i++)
			file << "-";
		file << "\n - " << a.first << "() => minimum: " << a.second.min << "ms"
							<< " | average: " << a.second.av << "ms"
							<< " | maximum: " << a.second.max << "ms" << std::endl;
	}
	for (size_t i = 0; i < 15; i++)
		file << "-";
	file << std::endl;
}
/// ---