#include "FirstApp.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
	Minimal::FirstApp renderer{};
	
	try
	{
		renderer.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}