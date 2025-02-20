#include "config.hpp"

bool VERBOSE = false;
bool SHOW_TOOLTIP = true;

static void	printUsage() {
	cout << BGreen << "=== ft_vox by DailyWind & HaSYxD ===" << ResetColor << endl;
	cout << "Usage : ./ft_vox [flags] [seed]" << endl;
	cout << endl;
	cout << "> Flags :" << endl;
	cout << "\t-h, --help\t\tPrint this message" << endl;
	cout << "\t-v, --verbose\t\tEnable verbose mode" << endl;
	cout << "\t-t, --no-tooltip\tDisable the commands tooltip" << endl;
	cout << endl;
	cout << "> Seed : Any unsigned long integer (0 by default = random)" << endl;
	cout << BGreen << "====================================" << ResetColor << endl;

	exit(EXIT_SUCCESS);
}

// Handle the flags and return the seed
uint64_t	flagHandler(int argc, char **argv) {
	uint64_t seed = 0; // Random seed by default

	for (int i = 1; i < argc; i++) {
		string arg = argv[i];

		if      (arg == "-h" || arg == "--help") 		printUsage();
		else if (arg == "-v" || arg == "--verbose")		VERBOSE = true;
		else if (arg == "-t" || arg == "--no-tooltip")	SHOW_TOOLTIP = false;

		else {
			if (i == argc - 1) {
				try { seed = stoull(arg); continue; }
				catch(const exception& e) { /* Stay 0 if fail */ }
			}
			cerr << BYellow << "Unknown flag : " << arg << ResetColor << endl;
		}
	}

	return seed;
}