#include "config.hpp"

bool VERBOSE = true;

// Print the message on the standard output if VERBOSE is set to true
void	printVerbose(const string &message) {
	if (VERBOSE)
		cout << message;
}