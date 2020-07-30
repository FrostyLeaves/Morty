#include "MIRenderTarget.h"
#include "MRenderPass.h"

MIRenderTarget::MIRenderTarget()
	: m_RenderPass(this) //TODO ≤ª”≈—≈
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkCommandBuffers.fill(VK_NULL_HANDLE);
#endif
}
