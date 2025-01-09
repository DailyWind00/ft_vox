#include "config.hpp"

int main(int argc, char **argv) {
	(void)argc; (void)argv;

	try {
		Window window(100, 100, 800, 600, "ft_vox");
		
		Rendering(window);
	}
	catch(const exception& e) {
		cerr << BRed <<  "Critical Error : " << e.what() << ResetColor <<'\n';
		exit(EXIT_FAILURE);
	}
}
