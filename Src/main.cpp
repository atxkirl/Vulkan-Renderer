#include <iostream>
#include "Renderer.h"
#include "VulkanRenderer.h"

//#define USE_OLD

int main()
{
#ifdef USE_OLD
	try
	{
		Renderer renderer;
		renderer.Initialize();
		renderer.Update();
		renderer.Shutdown();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#else
	try
	{
		MeowRenderer renderer;
		renderer.Init();
		renderer.Update();
		renderer.Release();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#endif

	return 0;
}
