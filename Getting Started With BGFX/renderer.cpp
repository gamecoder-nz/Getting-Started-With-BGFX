#include "renderer.h"

#include <print>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <backends/imgui_impl_bgfx.h>

#define _CRT_SECURE_NO_WARNINGS
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

	uint16_t pyramidIndices[] =
	{
		0,1,2,  // Front
		0,2,3,  // Right
		0,3,4,  // Back
		0,4,1,  // Left
		1,4,3,  // Bottom
		1,3,2,  // Bottom
	};

	m_PyramidIndexBuffer = bgfx::createIndexBuffer(
		bgfx::copy(pyramidIndices, sizeof(pyramidIndices) * 2)
	);

	if (bgfx::isValid(m_PyramidIndexBuffer) == false)
	{
		std::println("Could not create pyramic index buffer");
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

	Vertex pyramidVertices[] = {
		{ {  0.0f,  50.0f,  0.0f}, 0xff0000ff  , { 0.0f, 0.0f }},
		{ { -50.0f, -50.0f, 50.0f }, 0xff00ff00  , { 0.0f, 0.0f } },
		{ { 50.0f, -50.0f,  50.0f }, 0xffff0000  , { 0.0f, 0.0f } },
		{ { 50.0f, -50.0f, -50.0f }, 0xffffff00  , { 0.0f, 0.0f } },
		{ { -50.0f, -50.0f, -50.0f }, 0xff00ffff  , { 0.0f, 0.0f } }
	};

	m_PyramidVertexBuffer = bgfx::createVertexBuffer(bgfx::copy(pyramidVertices, sizeof(pyramidVertices)), m_VertexLayout);

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
	m_PerspectiveProjection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 1000.0f);

	m_FrameBuffer = new FrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

	m_NDCVertices.push_back({ {-1.0f, 1.0f, 0.0f}, 0xffffffff, {0.0f, 0.0f} });
	m_NDCVertices.push_back({ {1.0f, 1.0f, 0.0f}, 0xffffffff, {1.0f, 0.0f} });
	m_NDCVertices.push_back({ {1.0f, -1.0f, 0.0f}, 0xffffffff, {1.0f, 1.0f} });
	m_NDCVertices.push_back({ {-1.0f, -1.0f, 0.0f}, 0xffffffff, {0.0f, 1.0f} });
	m_NDCVertexBuffer = bgfx::createDynamicVertexBuffer(bgfx::makeRef(m_NDCVertices.data(), sizeof(Vertex) * m_NDCVertices.size()), m_VertexLayout);

	ImGui_ImplBgfx_Init(IMGUI_VIEW);

	m_ReadValueDestination = bgfx::createTexture2D(1, 1, false, 0, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);
	m_Frame = 0;
	m_ReadValueReadyFrame = 0xffffffff;

	FT_Init_FreeType(&m_FreeTypeLibrary);
	FT_New_Face(m_FreeTypeLibrary, "January Night.ttf", 0, &m_FontFace);
	CreateFontTexture();
}

void Renderer::Begin()
{
	bgfx::reset(SCREEN_WIDTH, SCREEN_HEIGHT, BGFX_RESET_VSYNC);
	m_Stencil = BGFX_STENCIL_NONE;

	m_Vertices.clear();
	m_RenderBatches.clear();

	bgfx::setViewClear(THREE_D_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00ffffff);
	bgfx::setViewRect(THREE_D_VIEW, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	bgfx::setViewTransform(THREE_D_VIEW, glm::value_ptr(m_View), glm::value_ptr(m_PerspectiveProjection));
	bgfx::setViewFrameBuffer(THREE_D_VIEW, m_FrameBuffer->GetFrameBufferHandle());

	bgfx::setViewRect(TWO_D_VIEW, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	bgfx::setViewClear(TWO_D_VIEW, BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00FFFFFF);
	bgfx::setViewTransform(TWO_D_VIEW, glm::value_ptr(m_View), glm::value_ptr(m_Projection));
	bgfx::setViewFrameBuffer(TWO_D_VIEW, m_FrameBuffer->GetFrameBufferHandle());

	bgfx::setViewRect(RENDER_VIEW, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	bgfx::setViewClear(RENDER_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xFFFFFFFF);

	bgfx::touch(TWO_D_VIEW);
	bgfx::touch(THREE_D_VIEW);

	ImGui_ImplBgfx_NewFrame();
}

void Renderer::DrawPyramid(glm::vec3 position, float rotation)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 1.0f, .0f));
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_ALPHA | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);
	bgfx::setStencil(m_Stencil);
	bgfx::setTransform(glm::value_ptr(transform));
	bgfx::setVertexBuffer(0, m_PyramidVertexBuffer);
	bgfx::setIndexBuffer(m_PyramidIndexBuffer, 0, 18);
	bgfx::setTexture(0, m_Uniform, m_WhiteImage->GetTextureHandle());
	bgfx::submit(THREE_D_VIEW, m_ShaderProgram->GetProgramHandle());
}


void Renderer::DrawQuad(glm::vec3 position, float rotation, uint32_t color, glm::vec3 scale, uint32_t depth)
{
	RenderBatch batch;
	batch.Stencil = m_Stencil;
	batch.Depth = depth;
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


void Renderer::DrawImage(Image* image, glm::vec3 position, float rotation, uint32_t color, uint32_t depth)
{
	RenderBatch batch;
	batch.Stencil = m_Stencil;
	batch.Depth = depth;
	batch.StartIndex = m_Vertices.size() / 4 * 6;
	batch.NumberOfIndices = 6;
	batch.Texture = image->GetTextureHandle();
	batch.Transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	m_RenderBatches.push_back(batch);

	m_Vertices.push_back({ {-50.0f, 50.0f, 0.0f}, color, {1.0, 1.0f} });
	m_Vertices.push_back({ {50.0f, 50.0f, 0.0f}, color, {0.0, 1.0f} });
	m_Vertices.push_back({ {50.0f, -50.0f, 0.0f}, color, {0.0, 0.0f} });
	m_Vertices.push_back({ {-50.0f, -50.0f, 0.0f}, color, {1.0, 0.0f} });
}

void Renderer::DrawText(glm::vec3 position, float rotation, uint32_t color, const char* text)
{
	std::string textString = text;

	RenderBatch batch;
	batch.Depth = 0;
	batch.StartIndex = m_Vertices.size() / 4 * 6;
	batch.NumberOfIndices = textString.size() * 6;
	batch.Texture = m_FontTexture;
	batch.Transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0, 0.0, 1.0f));
	batch.Stencil = m_Stencil;

	float x = 0.0f;
	m_RenderBatches.push_back(batch);

	for (char c : textString)
	{
		GlyphMetrics& glyphMetrics = m_GlyphMetrics[c];
		m_Vertices.push_back({ {x + glyphMetrics.Offset.x                            , position.y, 0.0f},                             color,  glm::vec2(glyphMetrics.TextureCoordinates0.x, glyphMetrics.TextureCoordinates1.y) });
		m_Vertices.push_back({ {x + glyphMetrics.Offset.x + glyphMetrics.Dimensions.x, position.y, 0.0f},                             color,  glm::vec2(glyphMetrics.TextureCoordinates1.x, glyphMetrics.TextureCoordinates1.y) });
		m_Vertices.push_back({ {x + glyphMetrics.Offset.x + glyphMetrics.Dimensions.x, position.y + glyphMetrics.Dimensions.y, 0.0f}, color,  glm::vec2(glyphMetrics.TextureCoordinates1.x, glyphMetrics.TextureCoordinates0.y) });
		m_Vertices.push_back({ {x + glyphMetrics.Offset.x                            , position.y + glyphMetrics.Dimensions.y, 0.0f}, color,  glm::vec2(glyphMetrics.TextureCoordinates0.x, glyphMetrics.TextureCoordinates0.y) });

		x += glyphMetrics.AdvanceWidth;
	}
}

void Renderer::WriteToStencil()
{
	m_Stencil = BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE | BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(0xff);
}

void Renderer::EnableStencilTest()
{
	m_Stencil = BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE | BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(0xff);
}

void Renderer::DisableStencil()
{
	m_Stencil = BGFX_STENCIL_NONE;
}

void Renderer::SetPostProcessingColor(uint32_t color)
{
	m_NDCVertices[0].Color = color;
	m_NDCVertices[1].Color = color;
	m_NDCVertices[2].Color = color;
	m_NDCVertices[3].Color = color;

	bgfx::update(m_NDCVertexBuffer, 0, bgfx::makeRef(m_NDCVertices.data(), m_NDCVertices.size() * sizeof(Vertex)));
}

void Renderer::ReadColor(uint32_t x, uint32_t y)
{
	bgfx::blit(BLIT_VIEW, m_ReadValueDestination, 0, 0, m_FrameBuffer->GetColorAttachmentHandle(), x, y, 1, 1);
	m_ReadValueReadyFrame = bgfx::readTexture(m_ReadValueDestination, &m_ReadValue);
}

std::optional<uint32_t> Renderer::GetReadColor()
{
	if (m_Frame == m_ReadValueReadyFrame)
	{
		return m_ReadValue;
	}

	return std::nullopt;
}

void Renderer::Render()
{
	if (m_Vertices.size() > 0)
	{
		bgfx::update(m_VertexBuffer, 0, bgfx::makeRef(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex)));
	}

	for (RenderBatch& batch : m_RenderBatches)
	{
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_ALPHA | BGFX_STATE_WRITE_Z);
		bgfx::setStencil(batch.Stencil);
		bgfx::setTransform(glm::value_ptr(batch.Transform));
		bgfx::setVertexBuffer(0, m_VertexBuffer);
		bgfx::setIndexBuffer(m_IndexBuffer, batch.StartIndex, batch.NumberOfIndices);
		bgfx::setTexture(0, m_Uniform, batch.Texture);
		bgfx::submit(TWO_D_VIEW, m_ShaderProgram->GetProgramHandle(), batch.Depth);
	}

	bgfx::setVertexBuffer(0, m_NDCVertexBuffer);
	bgfx::setIndexBuffer(m_IndexBuffer, 0, 6);
	bgfx::setTexture(0, m_Uniform, m_FrameBuffer->GetColorAttachmentHandle());
	bgfx::submit(RENDER_VIEW, m_ShaderProgram->GetProgramHandle());

	ImGui::Render();
	ImGui_ImplBgfx_RenderDrawData(ImGui::GetDrawData());

	m_Frame = bgfx::frame();
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

void Renderer::CreateFontTexture()
{
	const std::string textToRender = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>? ";
	FT_Set_Char_Size(m_FontFace, 100 * 64, 0, 96, 0);

	uint32_t maxWidth = 0, maxHeight = 0, rowWidth = 0;

	for (uint32_t index = 0; index < textToRender.length(); index++)
	{
		if (index > 0 && index % 10 == 0)
		{
			maxWidth = std::max(maxWidth, rowWidth);
			rowWidth = 0;
		}

		FT_UInt glyphIndex = FT_Get_Char_Index(m_FontFace, textToRender[index]);
		FT_Load_Glyph(m_FontFace, glyphIndex, 0);
		maxHeight = std::max(maxHeight, m_FontFace->glyph->bitmap.rows);
		rowWidth += m_FontFace->glyph->bitmap.width;
	}

	uint32_t totalWidth = maxWidth;
	uint32_t totalHeight = maxHeight * 10;

	std::vector<uint32_t> imageData;
	imageData.resize(totalWidth * totalHeight);
	std::fill(imageData.begin(), imageData.end(), 0x00ffffff);

	uint32_t xOffset = 0;
	uint32_t yOffset = 0;

	for (uint32_t index = 0; index < textToRender.length(); index++)
	{
		if (index > 0 && index % 10 == 0)
		{
			yOffset += maxHeight;
			xOffset = 0;
		}

		FT_UInt glyphIndex = FT_Get_Char_Index(m_FontFace, textToRender[index]);
		FT_Load_Glyph(m_FontFace, glyphIndex, 0);
		FT_Render_Glyph(m_FontFace->glyph, FT_RENDER_MODE_NORMAL);

		GlyphMetrics& glyphMetrics = m_GlyphMetrics[textToRender[index]];
		glyphMetrics.Offset.x = m_FontFace->glyph->bitmap_left;
		glyphMetrics.Offset.y = m_FontFace->glyph->bitmap_top;
		glyphMetrics.Dimensions.x = m_FontFace->glyph->bitmap.width;
		glyphMetrics.Dimensions.y = m_FontFace->glyph->bitmap.rows;
		glyphMetrics.AdvanceWidth = m_FontFace->glyph->advance.x >> 6;
		glyphMetrics.TextureCoordinates0.x = (float)xOffset / (float)totalWidth;
		glyphMetrics.TextureCoordinates0.y = (float)yOffset / (float)totalHeight;
		glyphMetrics.TextureCoordinates1.x = glyphMetrics.TextureCoordinates0.x + (float)glyphMetrics.Dimensions.x / (float)totalWidth;
		glyphMetrics.TextureCoordinates1.y = glyphMetrics.TextureCoordinates0.y + (float)glyphMetrics.Dimensions.y / (float)totalHeight;

		for (uint32_t y = 0; y < m_FontFace->glyph->bitmap.rows; y++)
		{
			for (uint32_t x = 0; x < m_FontFace->glyph->bitmap.width; x++)
			{
				imageData[(y + yOffset) * totalWidth + xOffset + x] |= ((uint32_t)m_FontFace->glyph->bitmap.buffer[y * m_FontFace->glyph->bitmap.width + x] << 24);
			}
		}

		xOffset += m_FontFace->glyph->bitmap.width;
	}

	m_FontTexture = bgfx::createTexture2D(
		(uint16_t)totalWidth,
		(uint16_t)totalHeight,
		false,
		1,
		bgfx::TextureFormat::RGBA8,
		BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP,
		bgfx::copy(imageData.data(), totalWidth * totalHeight * 4)
	);

	stbi_write_png("text.png", totalWidth, totalHeight, 4, imageData.data(), totalWidth * 4);
}
