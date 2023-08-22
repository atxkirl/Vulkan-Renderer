#include <iostream>
#include "Renderer.h"

int main()
{
	Renderer renderer;
	try
	{
		renderer.Initialize();
		renderer.Update();
		renderer.Shutdown();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}