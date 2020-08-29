#include "MIRenderTarget.h"
#include "MRenderPass.h"

MIRenderTarget::MIRenderTarget()
	: m_RenderPass(this) //TODO ≤ª”≈—≈
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkCommandBuffers.fill(VK_NULL_HANDLE);
	m_aVkRenderFinishedSemaphore.fill(VK_NULL_HANDLE);
	m_aVkRenderFinishedEvent.fill(VK_NULL_HANDLE);
#endif
}

MFrameBuffer::MFrameBuffer()
	: vBackTextures()
	, pDepthTexture()
	, vkFrameBuffer(VK_NULL_HANDLE)
{

}
