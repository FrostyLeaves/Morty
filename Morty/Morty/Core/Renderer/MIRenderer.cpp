#include "MIRenderer.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MIRenderTarget.h"

MIRenderer::MIRenderer()
	: m_eRasterizerType(MERasterizerType::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
{

}

void MIRenderer::Render(MIRenderTarget* pRenderTarget)
{
	if (pRenderTarget)
	{
		Render(pRenderTarget, pRenderTarget->GetDepthTexture());
	}
}

void MIRenderer::Render(MIRenderTarget* pRenderTarget, MRenderDepthTexture* pDepthTexture)
{
	if (pRenderTarget)
	{
		ClearRenderTarget(pRenderTarget);

		RenderTargetPair rtp(pRenderTarget, pDepthTexture);

		m_vRenderTargets.push(rtp);
		RecoverRenderTarget(rtp);
		pRenderTarget->OnRender(this);
		m_vRenderTargets.pop();


		//恢复上一个渲染目标的状态
		if (!m_vRenderTargets.empty())
			RecoverRenderTarget(m_vRenderTargets.top());
	}
}
