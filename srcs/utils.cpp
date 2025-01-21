#include "config.hpp"

bool VERBOSE = true;

// Print the message on the standard output if VERBOSE is set to true
void	printVerbose(const string &message, bool newline) {
	if (VERBOSE) {
		cout << message;
		if (newline)
			cout << endl;
	}
}

bool	isNum(const std::string &str)
{
	for (size_t i = 0; i < str.size(); i++)
		if (!std::isdigit(str[i]))
			return (false);
	return (true);
}

std::vector<std::string>	split(const std::string &str, const char &c)
{
	std::vector<std::string>	words;
	std::string		buff;
	std::stringstream	ss(str);

	while (std::getline(ss >> std::ws, buff, c))
		words.push_back(buff);
	return (words);
}
