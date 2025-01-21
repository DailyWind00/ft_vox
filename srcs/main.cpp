#include "config.hpp"

typedef std::pair<std::string, void *>	flagArg;

std::vector<flagArg>	parseArgs(int argc, char **argv)
{
	std::vector<flagArg>	args;

	for (int i = 1; i < argc; i++) {
		std::vector<std::string>	spStr = split(argv[i], '=');

		if (spStr.size() != 2) {
			std::string	str = "Error" + std::string(argv[i]) + "is not a valid argument";
			printVerbose(str);
			continue ;
		}
		if (isNum(spStr[1])) {
			int	val = stoi(spStr[1]);

			args.push_back(flagArg(spStr[0], &val));
			continue ;
		}
		args.push_back(flagArg(spStr[0], &spStr[1][0]));
	}
	return (args);
}

int	main(int argc, char **argv)
{
	FlagHandler	fh;

	// fh.setFlag("--seed", "-s", &Noise::setSeed);
	//-fh.setFlag("--genModel", "-GM", &Noise::setSeed);
	
	// std::vector<flagArg>	args = parseArgs(argc, argv);

	// for (size_t i = 0; i < fh.getFlagNum(); i++)
	// 	fh.execFlagsFunction(args[i].first, args[i].second);

	try {
		Window window(100, 100, 800, 600, "ft_vox");
		
		Rendering(window);
	}
	catch(const exception& e) {
		cerr << BRed <<  "Critical Error : " << e.what() << ResetColor <<'\n';
		exit(EXIT_FAILURE);
	}
}
