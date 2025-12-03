#include <print>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "image.h"

struct Vertex
{
	glm::vec3 Position;
	uint32_t Color;
	glm::vec2 TextureCoordinates;
};

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

	bgfx::VertexLayout vertexLayout;
	vertexLayout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();

	Vertex vertices[4];
	vertices[0] = { {-0.28f, 0.5f, 0.0f}, 0xffff00ff, {0.0, 0.0} };
	vertices[1] = { {0.28f, 0.5f, 0.0f}, 0xffff00ff, {1.0f, 0.0f} };
	vertices[2] = { {0.28f, -0.5f, 0.0f}, 0xffff00ff, {1.0f, 1.0f} };
	vertices[3] = { {-0.28f, -0.5f, 0.0f}, 0xffff00ff, {0.0f, 1.0f} };

	bgfx::VertexBufferHandle vertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(Vertex) * 4), vertexLayout);

	if (bgfx::isValid(vertexBuffer) == false)
	{
		std::println("Could not create vertex buffer");
		return 1;
	}

	ShaderProgram* shaderProgram = new ShaderProgram("vertex.bin", "frag.bin");
	Image* image = new Image("texture.jpg");
	uint32_t white = 0xffffffff;
	Image* whiteImage = new Image(&white, 1, 1);
	bgfx::UniformHandle uniform = bgfx::createUniform("textureColor", bgfx::UniformType::Sampler);

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
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00FFFFFF);
		bgfx::setTexture(0, uniform, whiteImage->GetTextureHandle());
		bgfx::setVertexBuffer(0, vertexBuffer);
		bgfx::setIndexBuffer(indexBuffer);
		bgfx::submit(0, shaderProgram->GetProgramHandle());

		bgfx::frame();
	}

	bgfx::destroy(indexBuffer);
	bgfx::destroy(vertexBuffer);
	delete shaderProgram;
	delete image;
	delete whiteImage;
	bgfx::destroy(uniform);
	bgfx::shutdown();
	SDL_DestroyWindow(window);
	return 0;
}