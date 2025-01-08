/// Class independant system includes
# include <iostream>
# include <sstream>
# include <fstream>

# include "Profiler.hpp"

Profiler::Profiler() {}
Profiler::~Profiler() {}

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

void	Profiler::printLog(const std::string &funcName)
{
	auto	a = this->_logs[funcName];

	std::cout << "\n - " << funcName << "() => minimum: " << a.min << "ms"
							<< " | average: " << a.av << "ms"
						       	<< " | maximum: " << a.max << "ms" << std::endl;
}

void	Profiler::logToFile()
{
	if (!this->_logs.size())
		std::cout << "Profiler is not holding any data" << std::endl;

	std::ofstream	file("out.logs");

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
