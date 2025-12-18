#pragma once

#include <print>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.h"
#include "image.h"
#include "frame_buffer.h"

struct Vertex
{
	glm::vec3 Position;
	uint32_t Color;
	glm::vec2 TextureCoordinates;
};

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

struct RenderBatch
{
	glm::mat4 Transform;
	bgfx::TextureHandle Texture;
	uint32_t StartIndex;
	uint32_t NumberOfIndices;

};

class Renderer
{
public:
	Renderer();
	void Initialize(SDL_Window* window);
	void Begin();
	void DrawQuad(glm::vec3 position, float rotation, uint32_t color, glm::vec3 scale);
	void DrawImage(Image* image, glm::vec3 position, float rotation, uint32_t color);
	void Render();
	void Shutdown();

private:
	bgfx::IndexBufferHandle m_IndexBuffer;
	bgfx::VertexLayout m_VertexLayout;
	bgfx::DynamicVertexBufferHandle m_VertexBuffer;
	bgfx::UniformHandle m_Uniform;
	Image* m_WhiteImage;
	ShaderProgram* m_ShaderProgram;
	std::vector<Vertex> m_Vertices;
	std::vector<RenderBatch> m_RenderBatches;
	FrameBuffer* m_FrameBuffer;

	bgfx::VertexBufferHandle m_NDCVertexBuffer;
	std::vector<Vertex> m_NDCVertices;

	glm::vec3 m_At;
	glm::vec3 m_Camera;
	glm::mat4 m_View;
	glm::mat4 m_Projection;
};