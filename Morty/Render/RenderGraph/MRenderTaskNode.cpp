#include "MRenderTaskNode.h"

#include "MEngine.h"
#include "MTexture.h"
#include "MRenderSystem.h"
#include "MRenderTaskNodeOutput.h"

MORTY_CLASS_IMPLEMENT(MRenderTaskNode, MTaskNode)

MRenderTaskNode::MRenderTaskNode()
	: MTaskNode()
{

}

MRenderTaskNode::~MRenderTaskNode()
{

}

MRenderTaskNodeOutput* MRenderTaskNode::AppendOutput()
{
	return MTaskNode::AppendOutput<MRenderTaskNodeOutput>();
}

void MRenderTaskNode::OnCompile()
{
	Super::OnCompile();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pDevice = pRenderSystem->GetDevice();
	//chean Renderpass.
	m_renderpass.DestroyBuffer(pDevice);
	m_renderpass.m_vBackDesc = {};
	m_renderpass.m_vBackTextures = {};
	m_renderpass.m_pDepthTexture = nullptr;
	m_renderpass.m_DepthDesc = {};

	for (size_t opIdx = 0; opIdx < m_vOutput.size(); ++opIdx)
	{
		if (MRenderTaskNodeOutput* pOutput = m_vOutput[opIdx]->DynamicCast<MRenderTaskNodeOutput>())
		{
			if (MTexture* pRenderTexture = nullptr)
			{

				if (pRenderTexture->GetRenderUsage() == METextureRenderUsage::ERenderBack)
				{
					m_renderpass.m_vBackTextures.push_back(pRenderTexture);

					m_renderpass.m_vBackDesc.push_back({});
					m_renderpass.m_vBackDesc.back().bClearWhenRender = pOutput->GetClear();
					m_renderpass.m_vBackDesc.back().cClearColor = pOutput->GetClearColor();
				}
				else if (pRenderTexture->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
				{
					m_renderpass.m_pDepthTexture = pRenderTexture;

					m_renderpass.m_DepthDesc.bClearWhenRender = pOutput->GetClear();
				}
				else
				{
					//error
					GetEngine()->GetLogger()->Error("MRenderTaskNode::OnCompile() unknow texture usage.");
				}

			}
			else
			{
				//error.
				GetEngine()->GetLogger()->Error("MRenderTaskNode::OnCompile() texture is null.");
			}
		}
	}


	m_renderpass.GenerateBuffer(pDevice);
}
