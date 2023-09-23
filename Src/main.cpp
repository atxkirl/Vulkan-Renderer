#include <iostream>

#define USE_VER 0

#include "Renderer.h"
#include "VulkanRenderer.h"

int main()
{
#if USE_VER == 0
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
#elif USE_VER == 1
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