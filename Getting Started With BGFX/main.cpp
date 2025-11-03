#include <print>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::println("Couldn't initialize SDL: {}", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Getting Started With BGFX", SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	bgfx::Init init;
	init.resolution.width = SCREEN_WIDTH;
	init.resolution.height = SCREEN_HEIGHT;
	init.platformData.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

	if (bgfx::init(init) == false)
	{
		std::println("Could not initialize BGFX");
		return 1;
	}

	uint16_t indices[6] = { 3, 2, 0, 2, 1, 0 };
	bgfx::IndexBufferHandle indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(indices, 12));

	if (bgfx::isValid(indexBuffer) == false)
	{
		std::println("Could not create index buffer");
		return 1;
	}

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

		bgfx::setViewRect(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR, 0x00FFFFFF);

		bgfx::touch(0);

		bgfx::frame();
	}

	bgfx::destroy(indexBuffer);
	bgfx::shutdown();
	SDL_DestroyWindow(window);
	return 0;
}