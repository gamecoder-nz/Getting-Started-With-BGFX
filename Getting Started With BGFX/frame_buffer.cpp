#include "frame_buffer.h"

FrameBuffer::FrameBuffer(uint32_t width, uint32_t height)
{
	m_ColorAttachment = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);
	m_DepthStencilAttachment = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT);

	bgfx::Attachment m_Attachments[2];
	m_Attachments[0].init(m_ColorAttachment);
	m_Attachments[1].init(m_DepthStencilAttachment);

	m_FrameBufferHandle = bgfx::createFrameBuffer(2, &m_Attachments[0], true);
}

FrameBuffer::~FrameBuffer()
{
	bgfx::destroy(m_FrameBufferHandle);
}