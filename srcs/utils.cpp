#include "config.hpp"

// Print player controls on the standard output
void	printControls() {
	if (!SHOW_TOOLTIP)
		return;

	cout << BLightBlue << "=== Commands ===\n" << ResetColor;
	cout << "WASD\t\tMove the camera\n";
	cout << "Space\t\tMove up\n";
	cout << "Ctrl\t\tMove down\n";
	cout << endl;
	cout << "Mouse\t\tLook around\n";
	cout << "Shift\t\tSprint\n";
	cout << "Esc\t\tClose the window\n";
	cout << BLightBlue << "================\n" << ResetColor;
}

// Print the message on the standard output if VERBOSE is set to true
void	printVerbose(const string &message) {
	if (VERBOSE)
		cout << message;
}