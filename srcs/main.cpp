#include "config.hpp"

int	main(int argc, char **argv) {

	uint64_t	seed;
	if (argc > 1)
		seed = flagHandler(argc, argv);

	printControls();

	try {
		Window window(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "ft_vox");
		
		Rendering(window, seed);
	}
	catch(const exception& e) {
		cerr << BRed <<  "Critical Error : " << e.what() << ResetColor <<'\n';
		exit(EXIT_FAILURE);
	}
}
