#include <print>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::println("Couldn't initialize SDL: {}", SDL_GetError());
	}

	SDL_Window* window = SDL_CreateWindow("Getting Started With BGFX", SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	bool running = true;
	SDL_Event event;

	while (running == true)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}
	}

	SDL_DestroyWindow(window);
	return 0;
}