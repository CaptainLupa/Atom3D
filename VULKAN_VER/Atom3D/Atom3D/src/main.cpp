#include <iostream>

#include "AtomCore.hpp"
//#include "Tutorialbase.cpp"

int main() {
	Atom::AtomCore engine;
	//HelloTriangleApplication app;

	engine.init();

	try {
		engine.run();
		//app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	engine.cleanup();

	return 0;
}