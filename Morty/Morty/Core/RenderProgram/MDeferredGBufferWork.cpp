#include "MDeferredGBufferWork.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MSkeleton.h"
#include "MMaterial.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "MMaterialResource.h"
#include "Model/MMeshResource.h"
#include "Model/MIMeshInstance.h"


#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

#include "MDeferredRenderProgram.h"

M_OBJECT_IMPLEMENT(MDeferredGBufferWork, MObject)

MDeferredGBufferWork::MDeferredGBufferWork()
    : MObject()
    , m_pRenderProgram(nullptr)
	, m_FrameParamSet()
	, m_pScreenDrawMesh(nullptr)
	, m_pLightningMaterial(nullptr)
{
}

MDeferredGBufferWork::~MDeferredGBufferWork()
{
}

void MDeferredGBufferWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeMesh();
	InitializeMaterial();
	InitializeRenderGraph();
	InitializeShaderParamSet();
}

void MDeferredGBufferWork::Release()
{
	ReleaseShaderParamSet();
	ReleaseRenderGraph();
	ReleaseMaterial();
	ReleaseMesh();
}

void MDeferredGBufferWork::InitializeShaderParamSet()
{
	m_FrameParamSet.InitializeShaderParamSet(GetEngine());
}

void MDeferredGBufferWork::ReleaseShaderParamSet()
{
	m_FrameParamSet.ReleaseShaderParamSet(GetEngine());
}

void MDeferredGBufferWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	const std::vector<MString> aTextureName = {
		"Base_Metallic",
		"Albedo_AmbientOcc",
		"Normal_Roughness"
	};

	MRenderGraphNode* pGBufferNode = pRenderGraph->AddRenderGraphNode("GBuffer Node");
	MRenderGraphNode* pLightningNode = pRenderGraph->AddRenderGraphNode("Lightning Node");

	MRenderGraphNodeInput* pInputNode = pGBufferNode->AppendInput();

	for (const MString& strTextureName : aTextureName)
	{
		MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->FindRenderGraphTexture(strTextureName);
		if (nullptr == pOutputTargetTexture)
		{
			pOutputTargetTexture = pRenderGraph->AddRenderGraphTexture(strTextureName);
			pOutputTargetTexture->SetUsage(METextureUsage::ERenderBack);
			pOutputTargetTexture->SetLayout(METextureLayout::ERGBA8);
			pOutputTargetTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pOutputTargetTexture->SetSize(Vector2(1.0f, 1.0f));
		}

		MRenderGraphNodeOutput* pOutputTarget = pGBufferNode->AppendOutput();
		pOutputTarget->SetClear(true);
		pOutputTarget->SetClearColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		pOutputTarget->SetRenderTexture(pOutputTargetTexture);

		pOutputTarget->LinkTo(pLightningNode->AppendInput());
	}


	MRenderGraphTexture* pOutputDepthTexture = pRenderGraph->FindRenderGraphTexture("Output Depth");
	if (nullptr == pOutputDepthTexture)
	{
		pOutputDepthTexture = pRenderGraph->AddRenderGraphTexture("Output Depth");
		pOutputDepthTexture->SetUsage(METextureUsage::ERenderDepth);
		pOutputDepthTexture->SetLayout(METextureLayout::EDepth);
		pOutputDepthTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pOutputDepthTexture->SetSize(Vector2(1.0f, 1.0f));
	}

	MRenderGraphNodeOutput* pOutputDepth = pGBufferNode->AppendOutput();
	pOutputDepth->SetClear(true);
	pOutputDepth->SetRenderTexture(pOutputDepthTexture);

	pOutputDepth->LinkTo(pLightningNode->AppendInput());


	if (MRenderGraphNodeOutput* pLightningOutput = pLightningNode->AppendOutput())
	{
		MRenderGraphTexture* pOutputTexture = pRenderGraph->AddRenderGraphTexture("Lightning Output Texture");
		pOutputTexture->SetUsage(METextureUsage::ERenderBack);
		pOutputTexture->SetLayout(METextureLayout::ERGBA8);
		pOutputTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pOutputTexture->SetSize(Vector2(1.0f, 1.0f));

		pLightningOutput->SetClear(true);
		pLightningOutput->SetClearColor(MColor::Black_T);
		pLightningOutput->SetRenderTexture(pOutputTexture);

		pRenderGraph->SetFinalOutput(pLightningOutput);
	}

	pGBufferNode->BindUpdateFunction(std::bind(&MDeferredGBufferWork::RenderUpdate, this, std::placeholders::_1));
	pGBufferNode->BindRenderFunction(std::bind(&MDeferredGBufferWork::Render, this, std::placeholders::_1));
	pLightningNode->BindRenderFunction(std::bind(&MDeferredGBufferWork::Lightning, this, std::placeholders::_1));




	MShaderParamSet* pShaderParamSet = m_pLightningMaterial->GetMaterialParamSet();

	for (size_t i = 0; i < 4; ++i)
	{
		uint32_t nOutputIdx = pShaderParamSet->m_vTextures[i]->unBinding;
		pShaderParamSet->m_vTextures[i]->pTexture = pGBufferNode->GetOutput(nOutputIdx)->GetRenderTexture()->GetRenderTexture();
		pShaderParamSet->m_vTextures[i]->SetDirty();
	}

}

void MDeferredGBufferWork::ReleaseRenderGraph()
{

}

void MDeferredGBufferWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(MGlobal::DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MDeferredGBufferWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(MGlobal::DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MDeferredGBufferWork::InitializeMaterial()
{
	m_pLightningMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
	m_pLightningMaterial->AddRef();

	m_pLightningMaterial->LoadVertexShader("./Shader/post_process_basic.mvs");
	m_pLightningMaterial->LoadPixelShader("./Shader/model_deferred.mps");
}

void MDeferredGBufferWork::ReleaseMaterial()
{
	m_pLightningMaterial->SubRef();
	m_pLightningMaterial = nullptr;
}

void MDeferredGBufferWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MDeferredGBufferWork::RenderUpdate(MRenderGraphNode* pGraphNode)
{
	MDeferredRenderProgram* pRenderProgram = dynamic_cast<MDeferredRenderProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;
	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	m_FrameParamSet.UpdateShaderSharedParams(info);

}

void MDeferredGBufferWork::Render(MRenderGraphNode* pGraphNode)
{
	MDeferredRenderProgram* pRenderProgram = dynamic_cast<MDeferredRenderProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;
	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, info.pViewport->GetWidth(), info.pViewport->GetHeight()));

	DrawNormalMesh(info);

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MDeferredGBufferWork::Lightning(MRenderGraphNode* pGraphNode)
{
	MDeferredRenderProgram* pRenderProgram = dynamic_cast<MDeferredRenderProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;
	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, {
		pGraphNode->GetInputTexture(0)->GetRenderTexture(),
		pGraphNode->GetInputTexture(1)->GetRenderTexture(),
		pGraphNode->GetInputTexture(2)->GetRenderTexture(),
		pGraphNode->GetInputTexture(3)->GetRenderTexture()
	});

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, info.pViewport->GetWidth(), info.pViewport->GetHeight()));


	if (info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_pLightningMaterial))
	{
		info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, &m_FrameParamSet);
		info.pRenderer->DrawMesh(info.pPrimaryCommand, m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MDeferredGBufferWork::DrawNormalMesh(MRenderInfo& info)
{
	for (MMaterialGroup& group : info.vMaterialRenderGroup)
	{
		MMaterial* pMaterial = group.m_pMaterial;

		if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
			continue;

		info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, &m_FrameParamSet);

		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			DrawMeshInstance(info, pMeshIns);
		}
	}
}

void MDeferredGBufferWork::DrawMeshInstance(MRenderInfo& info, MIMeshInstance* pMeshInstance)
{
	if (MSkeletonInstance* pSkeletonIns = pMeshInstance->GetSkeletonInstance())
	{
		info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pSkeletonIns->GetShaderParamSet());
	}

	info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pMeshInstance->GetShaderMeshParamSet());
	info.pRenderer->DrawMesh(info.pPrimaryCommand, pMeshInstance->GetMesh());
}
