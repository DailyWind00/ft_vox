#include "config.hpp"

int	main(int argc, char **argv)
{
	(void)argc; (void)argv;

	try {
		Window window(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "ft_vox");
		
		Rendering(window);
	}
	catch(const exception& e) {
		cerr << BRed <<  "Critical Error : " << e.what() << ResetColor <<'\n';
		exit(EXIT_FAILURE);
	}
}
