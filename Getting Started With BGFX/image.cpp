#include "image.h"
#include <print>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image(const char* filename)
{
	int32_t width, height;
	uint8_t* data = stbi_load(filename, &width, &height, nullptr, STBI_rgb_alpha);

	m_TextureHandle = bgfx::createTexture2D(
		(uint16_t)width,
		(uint16_t)height,
		false,
		1,
		bgfx::TextureFormat::Enum::RGBA8,
		BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP,
		bgfx::copy(data, width * height * 4)
	);

	delete data;

	if (bgfx::isValid(m_TextureHandle) == false)
	{
		std::print("Unable to create image");
	}
}

Image::Image(void* data, uint16_t width, uint16_t height)
{
	m_TextureHandle = bgfx::createTexture2D(
		(uint16_t)width,
		(uint16_t)height,
		false,
		1,
		bgfx::TextureFormat::Enum::RGBA8,
		BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP,
		bgfx::copy(data, width * height * 4)
	);

	if (bgfx::isValid(m_TextureHandle) == false)
	{
		std::print("Unable to create image");
	}
}

Image::~Image()
{
	bgfx::destroy(m_TextureHandle);
}