#include "MBasicPostProcessRenderWork.h"

#include "MForwardRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"

MORTY_INTERFACE_IMPLEMENT(MBasicPostProcessRenderWork, ISinglePassRenderWork)

void MBasicPostProcessRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	m_pMaterial = CreateMaterial();
}

void MBasicPostProcessRenderWork::Release()
{
	Super::Release();
}

void MBasicPostProcessRenderWork::Render(const MRenderInfo& info)
{
	auto pCommand = info.pPrimaryRenderCommand;
	MIMesh* pScreenMesh = GetEngine()->FindGlobalObject<MMeshManager>()->GetScreenRect();
	
	pCommand->AddRenderToTextureBarrier(m_vBarrierTexture, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(&m_renderPass);

	const Vector2i n2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


	if (pCommand->SetUseMaterial(m_pMaterial))
	{
		pCommand->SetShaderPropertyBlock(GetRenderGraph()->GetFrameProperty()->GetPropertyBlock());

		pCommand->DrawMesh(pScreenMesh);
	}

	pCommand->EndRenderPass();
}

void MBasicPostProcessRenderWork::BindTarget()
{
	if (std::shared_ptr<MShaderPropertyBlock> pPropertyBlock = m_pMaterial->GetMaterialPropertyBlock())
	{
		for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
		{
			pPropertyBlock->SetTexture(MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE[nInputIdx], GetInputTexture(nInputIdx));
		}
	}

	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTarget());
}
