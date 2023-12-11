#include "MPostProcessGraphWalker.h"

#include "MPostProcessNode.h"
#include "Render/MRenderCommand.h"
#include "Render/MRenderPass.h"
#include "Material/MMaterial.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "TaskGraph/MTaskGraph.h"

MPostProcessGraphWalker::MPostProcessGraphWalker(MIRenderCommand* pRenderCommand, MIMesh* pScreenMesh, std::shared_ptr<IPropertyBlockAdapter> pFrameProperty)
    : m_pRenderCommand(pRenderCommand)
    , m_pScreenMesh(pScreenMesh)
    , m_pFrameProperty(pFrameProperty)
{
}

void MPostProcessGraphWalker::operator ()(MTaskGraph* pTaskGraph)
{
	if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile())
	{
		return;
	}

	std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

	for (MTaskNode* pCurrentNode : vNodes)
	{
		RecordCommand(pCurrentNode->DynamicCast<MPostProcessNode>());
	}
	
}

void MPostProcessGraphWalker::RecordCommand(MPostProcessNode* pNode)
{
	auto pCommand = m_pRenderCommand;

	auto pMaterial = pNode->GetMaterial();
	auto pRenderPass = pNode->GetRenderPass();

	auto vTextureParams = pMaterial->GetMaterialPropertyBlock()->m_vTextures;
	std::vector<MTexture*> vTextures(vTextureParams.size());

	std::transform(vTextureParams.begin(), vTextureParams.end(), vTextures.begin(), [](auto param) { return param->pTexture.get(); });
	pCommand->AddRenderToTextureBarrier(vTextures, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(pRenderPass);

	const Vector2i n2Size = pRenderPass->GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


	if (pCommand->SetUseMaterial(pMaterial))
	{
		if (m_pFrameProperty)
		{
			pCommand->SetShaderPropertyBlock(m_pFrameProperty->GetPropertyBlock());
		}

		pCommand->DrawMesh(m_pScreenMesh);
	}

	pCommand->EndRenderPass();

}
