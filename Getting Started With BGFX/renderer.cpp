#include "renderer.h"

#include <print>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer()
{

}

void Renderer::Initialize(SDL_Window* window)
{
	bgfx::Init init;
	init.resolution.width = SCREEN_WIDTH;
	init.resolution.height = SCREEN_HEIGHT;
	init.platformData.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

	if (bgfx::init(init) == false)
	{
		std::println("Could not initialize BGFX");
	}

	std::vector<uint16_t> indices;

	for (uint32_t index = 0; index < 100; index++)
	{
		indices.push_back(4 * index + 3);
		indices.push_back(4 * index + 2);
		indices.push_back(4 * index + 0);

		indices.push_back(4 * index + 2);
		indices.push_back(4 * index + 1);
		indices.push_back(4 * index + 0);

	}

	m_IndexBuffer = bgfx::createIndexBuffer(bgfx::copy(indices.data(), indices.size() * 2));
	if (bgfx::isValid(m_IndexBuffer) == false)
	{
		std::println("Could not create index buffer");
	}


	m_VertexLayout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();

	m_Vertices.resize(100);
	m_VertexBuffer = bgfx::createDynamicVertexBuffer(bgfx::makeRef(m_Vertices.data(), sizeof(Vertex) * m_Vertices.size()), m_VertexLayout);
	if (bgfx::isValid(m_VertexBuffer) == false)
	{
		std::println("Could not create vertex buffer");
	}

	m_Uniform = bgfx::createUniform("textureColor", bgfx::UniformType::Sampler);
	if (bgfx::isValid(m_Uniform) == false)
	{
		std::println("Could not create uniform");
	}

	uint32_t white = 0xffffffff;
	m_WhiteImage = new Image(&white, 1, 1);
	m_ShaderProgram = new ShaderProgram("vertex.bin", "frag.bin");

	m_Camera = { 0.0f, 0.0f, -1.0f };
	m_At = { 0.0f, 0.0f, 0.0f };

	m_View = glm::lookAt(m_Camera, m_At, { 0.0f, 1.0f, 0.0f });
	m_Projection = glm::ortho(-SCREEN_WIDTH / 2.0f, SCREEN_WIDTH / 2.0f, -SCREEN_HEIGHT / 2.0f, SCREEN_HEIGHT / 2.0f, -1000.0f, 1000.0f);
}

void Renderer::Begin()
{
	bgfx::reset(SCREEN_WIDTH, SCREEN_HEIGHT, BGFX_RESET_VSYNC);

	m_Vertices.clear();
	m_RenderBatches.clear();

	bgfx::setViewRect(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00FFFFFF);
	bgfx::setViewTransform(0, glm::value_ptr(m_View), glm::value_ptr(m_Projection));

	bgfx::touch(0);
}


void Renderer::DrawQuad(glm::vec3 position, float rotation, uint32_t color, glm::vec3 scale)
{
	RenderBatch batch;
	batch.StartIndex = m_Vertices.size() / 4 * 6;
	batch.NumberOfIndices = 6;
	batch.Texture = m_WhiteImage->GetTextureHandle();
	batch.Transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), scale);

	m_RenderBatches.push_back(batch);

	m_Vertices.push_back({ {-50.0f, 50.0f, 0.0f}, color, {0.0, 1.0f} });
	m_Vertices.push_back({ {50.0f, 50.0f, 0.0f}, color, {1.0, 1.0f} });
	m_Vertices.push_back({ {50.0f, -50.0f, 0.0f}, color, {1.0, 0.0f} });
	m_Vertices.push_back({ {-50.0f, -50.0f, 0.0f}, color, {0.0, 0.0f} });
}


void Renderer::DrawImage(Image* image, glm::vec3 position, float rotation, uint32_t color)
{
	RenderBatch batch;
	batch.StartIndex = m_Vertices.size() / 4 * 6;
	batch.NumberOfIndices = 6;
	batch.Texture = image->GetTextureHandle();
	batch.Transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	m_RenderBatches.push_back(batch);

	m_Vertices.push_back({ {-50.0f, 50.0f, 0.0f}, color, {0.0, 1.0f} });
	m_Vertices.push_back({ {50.0f, 50.0f, 0.0f}, color, {1.0, 1.0f} });
	m_Vertices.push_back({ {50.0f, -50.0f, 0.0f}, color, {1.0, 0.0f} });
	m_Vertices.push_back({ {-50.0f, -50.0f, 0.0f}, color, {0.0, 0.0f} });
}

void Renderer::Render()
{
	if (m_Vertices.size() > 0)
	{
		bgfx::update(m_VertexBuffer, 0, bgfx::makeRef(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex)));
	}

	for (RenderBatch& batch : m_RenderBatches)
	{
		bgfx::setTransform(glm::value_ptr(batch.Transform));
		bgfx::setVertexBuffer(0, m_VertexBuffer);
		bgfx::setIndexBuffer(m_IndexBuffer, batch.StartIndex, batch.NumberOfIndices);
		bgfx::setTexture(0, m_Uniform, batch.Texture);
		bgfx::submit(0, m_ShaderProgram->GetProgramHandle());
	}

	bgfx::frame();
}

void Renderer::Shutdown()
{
	bgfx::destroy(m_IndexBuffer);
	bgfx::destroy(m_VertexBuffer);
	delete m_ShaderProgram;
	delete m_WhiteImage;
	bgfx::destroy(m_Uniform);
	bgfx::shutdown();
}