#include "MIRenderTarget.h"

#include "MEngine.h"
#include "MRenderPass.h"

M_I_OBJECT_IMPLEMENT(MIRenderTarget, MObject)

MIRenderTarget::MIRenderTarget()
	: MObject()
	, m_pRenderProgram(nullptr)
	, m_v2Size(0, 0)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkCommandBuffers.fill(VK_NULL_HANDLE);
	m_aVkRenderFinishedSemaphore.fill(VK_NULL_HANDLE);
#endif
}

void MIRenderTarget::OnDelete()
{
	if (m_pRenderProgram)
	{
		m_pRenderProgram->DeleteLater();
		m_pRenderProgram = nullptr;
	}


	GetEngine()->GetDevice()->DestroyRenderTarget(this);

	Super::OnDelete();
}

uint32_t MIRenderTarget::GetFrameBufferIndex()
{
	return GetEngine()->GetRenderer()->GetFrameIndex();
}

MFrameBuffer::MFrameBuffer()
	: vBackTextures()
	, pDepthTexture()
	, vkFrameBuffer(VK_NULL_HANDLE)
{

}
