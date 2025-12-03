#pragma once

#include <bgfx/bgfx.h>

class Image
{
public:
	Image(const char* filename);
	Image(void* data, uint16_t width, uint16_t height);
	~Image();

	bgfx::TextureHandle GetTextureHandle() { return m_TextureHandle; }

private:
	bgfx::TextureHandle m_TextureHandle;
};