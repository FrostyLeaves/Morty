#include "MForwardPostProcessProgram.h"

#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MForwardHDRWork.h"
#include "MGaussianBlurWork.h"
#include "MForwardRenderWork.h"
#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardPostProcessProgram, MForwardRenderProgram)

MForwardPostProcessProgram::MForwardPostProcessProgram()
    : MForwardRenderProgram()
	, m_bHDR_Enable(false)
	, m_vPostProcessWork()
	, m_pScreenDrawMaterial(nullptr)
{
}

MForwardPostProcessProgram::~MForwardPostProcessProgram()
{
}

void MForwardPostProcessProgram::Render(MIRenderer* pRenderer, MViewport* pViewport)
{
	MForwardRenderProgram::Render(pRenderer, pViewport);
}

void MForwardPostProcessProgram::SetHighDynamicRangeEnable(const bool& bEnable)
{
	if (m_bHDR_Enable == bEnable)
		return;
	m_bHDR_Enable = bEnable;

}

void MForwardPostProcessProgram::Initialize()
{
    Super::Initialize();

	MForwardHDRWork* pHDRWork = GetEngine()->GetObjectManager()->CreateObject<MForwardHDRWork>();
	pHDRWork->Initialize(this);


	if (MRenderGraphNodeOutput* pFinalOutput = m_pRenderGraph->GetFinalOutput())
	{
		pFinalOutput->LinkTo(pHDRWork->GetInput());
		m_pRenderGraph->SetFinalOutput(pHDRWork->GetOutput());
	}

	m_vPostProcessWork.push_back(pHDRWork);
}

void MForwardPostProcessProgram::Release()
{

	for (MIPostProcessWork* pPostProcess : m_vPostProcessWork)
	{
		pPostProcess->DeleteLater();
		pPostProcess = nullptr;
	}

	m_vPostProcessWork.clear();

    Super::Release();
}

