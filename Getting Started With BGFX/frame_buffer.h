#pragma once

#include <bgfx/bgfx.h>

class FrameBuffer
{
public:
	FrameBuffer(uint32_t width, uint32_t height);
	~FrameBuffer();
	bgfx::FrameBufferHandle GetFrameBufferHandle() { return m_FrameBufferHandle; }
	bgfx::TextureHandle GetColorAttachmentHandle() { return m_ColorAttachment; }

private:
	bgfx::FrameBufferHandle m_FrameBufferHandle;
	bgfx::TextureHandle m_ColorAttachment;
	bgfx::TextureHandle m_DepthStencilAttachment;
};