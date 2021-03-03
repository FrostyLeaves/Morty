#include "MIRenderTarget.h"

#include "MEngine.h"
#include "MRenderPass.h"

M_I_OBJECT_IMPLEMENT(MIRenderTarget, MObject)

MIRenderTarget::MIRenderTarget()
	: MObject()
	, m_v2Size(0, 0)
	, m_pRenderProgram(nullptr)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
#endif
}

void MIRenderTarget::OnDelete()
{
	GetEngine()->GetDevice()->DestroyRenderTarget(this);

	Super::OnDelete();
}

MFrameBuffer::MFrameBuffer()
	: vBackTextures()
	, pDepthTexture()
	, m_aVkFrameBuffer()
{

}
