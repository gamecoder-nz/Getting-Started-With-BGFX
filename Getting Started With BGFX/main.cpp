#include "renderer.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <glm/gtc/type_ptr.hpp>

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::println("Couldn't initialize SDL: {}", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Getting Started With BGFX", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForOther(window);

	Renderer renderer;
	renderer.Initialize(window);

	Image* image = new Image("texture.jpg");
	bool running = true;
	SDL_Event event;
	float rotation = 0.0f;
	glm::vec4 postProcessingColor(1.0f);

	while (running == true)
	{
		ImGui_ImplSDL3_NewFrame();

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (event.button.x >= 0 && event.button.y >= 0 && event.button.x < SCREEN_WIDTH && event.button.y < SCREEN_HEIGHT)
					{
						renderer.ReadColor(event.button.x, event.button.y);
					}
				}
				break;
			}
		}

		renderer.Begin();
		renderer.DrawPyramid({ -100.0f, 50.0f, 200.0f }, rotation);
		renderer.DrawQuad({ -550.0f, 0.0f, 0.1f }, 0.0f, 0x7700ffff, { 1.0f, 1.0f, 1.0f }, 0);
		renderer.DrawQuad({ -500.0f, 0.0f, 0.2f }, 0.0f, 0xffff00ff, { 1.0f, 1.0f, 1.0f }, 1);

		renderer.DrawImage(image, { 0.0f, 200.0f, 0.0f }, rotation, 0xffffffff, 0);

		renderer.WriteToStencil();
		renderer.DrawQuad({ 500.0f, 0.0f, 0.0f }, 0.0f, 0xffff00ff, { 1.0f, 1.0f, 1.0 }, 0);
		renderer.EnableStencilTest();
		renderer.DrawQuad({ 500.0f, 0.0f, 0.0f }, 0.0f, 0xff00ffff, { 1.3f, 1.3f, 1.0 }, 0);
		renderer.DisableStencil();

		ImGui::NewFrame();
		ImGui::Begin("Window");

		if (ImGui::ColorPicker4("Post Processing Color", glm::value_ptr(postProcessingColor)) == true)
		{
			uint32_t newPostProcessingColor =
				((uint32_t)(postProcessingColor.a * 255.0f) << 24)
				| ((uint32_t)(postProcessingColor.b * 255.0f) << 16)
				| ((uint32_t)(postProcessingColor.g * 255.0f) << 8)
				| ((uint32_t)(postProcessingColor.r * 255.0f));

			renderer.SetPostProcessingColor(newPostProcessingColor);
		}

		ImGui::End();

		renderer.Render();
		rotation += 1.0f;
		rotation = std::fmod(rotation, 360.0f);

		std::optional<uint32_t> readColorOptional = renderer.GetReadColor();
		if (readColorOptional.has_value() == true)
		{
			uint32_t readValue = readColorOptional.value();

			std::println("R: {} G: {} B: {} A:{} ", (readValue & 0x000000ff), (readValue & 0x0000ff00) >> 8, (readValue & 0x00ff0000) >> 16, (readValue & 0xff000000) >> 24);
		}
	}

	renderer.Shutdown();
	SDL_DestroyWindow(window);
	return 0;
}