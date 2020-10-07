#include "MIRenderTarget.h"
#include "MRenderPass.h"

M_I_OBJECT_IMPLEMENT(MIRenderTarget, MObject)

MIRenderTarget::MIRenderTarget()
	: MObject()
	, m_pRenderProgram(nullptr)
	, m_v2Size(256, 256)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkCommandBuffers.fill(VK_NULL_HANDLE);
	m_aVkRenderFinishedSemaphore.fill(VK_NULL_HANDLE);
#endif
}

MFrameBuffer::MFrameBuffer()
	: vBackTextures()
	, pDepthTexture()
	, vkFrameBuffer(VK_NULL_HANDLE)
	, vkExtend()
{

}
