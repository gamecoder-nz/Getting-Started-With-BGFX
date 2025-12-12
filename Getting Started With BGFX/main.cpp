#include "renderer.h"


int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::println("Couldn't initialize SDL: {}", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Getting Started With BGFX", SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	Renderer renderer;
	renderer.Initialize(window);

	Image* image = new Image("texture.jpg");
	bool running = true;
	SDL_Event event;
	float rotation = 0.0f;

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

		renderer.Begin();
		renderer.DrawQuad({ 500.0f, 0.0f, 0.0f }, 0.0f, 0xff00ffff, { 1.3f, 1.3f, 1.0f });
		renderer.DrawImage(image, { 0.0f, 200.0f, 0.0f }, rotation, 0xffffffff);
		renderer.Render();
		rotation += 1.0f;
		rotation = std::fmod(rotation, 360.0f);
	}

	renderer.Shutdown();
	SDL_DestroyWindow(window);
	return 0;
}