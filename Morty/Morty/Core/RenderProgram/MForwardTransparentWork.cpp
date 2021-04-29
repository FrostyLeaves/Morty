#include "MForwardTransparentWork.h"
#include "MForwardRenderProgram.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MSkeleton.h"
#include "MResourceManager.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"

#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

#include "MForwardRenderWork.h"

#include "MRenderableMeshComponent.h"

M_OBJECT_IMPLEMENT(MForwardTransparentWork, MObject)

MForwardTransparentWork::MForwardTransparentWork()
	: MObject()
	, m_TransparentDrawMesh(true)
	, m_pDrawMeshMaterial(nullptr)
	, m_pDrawFillMaterial(nullptr)
	, m_pWhiteTexture(nullptr)
	, m_pBlackTexture(nullptr)
//	, m_pTransparentRenderTarget(nullptr)
	, m_v2TransparentTextureSize(0.0f, 0.0f)
	, m_aFrameParamSet()
{
	
}

MForwardTransparentWork::~MForwardTransparentWork()
{
}

void MForwardTransparentWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeMaterial();
	InitializeMesh();
	InitializeTexture();

	for (uint32_t i = 0; i < 2; ++i)
	{
		m_aFrameParamSet[i].InitializeShaderParamSet(GetEngine());
	}

	InitializeRenderGraph();

	
}

void MForwardTransparentWork::Release()
{
	for (uint32_t i = 0; i < 2; ++i)
	{
		m_aFrameParamSet[i].ReleaseShaderParamSet(GetEngine());
	}

	ReleaseTexture();
	ReleaseMesh();
	ReleaseMaterial();
}

void MForwardTransparentWork::Render(MRenderGraphNode* pGraphNode)
{
	MRenderInfo& info = *m_pRenderProgram->GetRenderInfo();

	if (!pGraphNode)
		return;

	if (info.vTransparentRenderGroup.empty())
		return;

	MRenderPass* pRenderPass = pGraphNode->GetRenderPass();
	if (!pRenderPass)
		return;

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));


	MRenderGraphTexture* pFrontTexture = pGraphNode->GetInput(1)->GetLinkedTexture();
	MRenderGraphTexture* pBackTexture = pGraphNode->GetInput(2)->GetLinkedTexture();

	info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, { pFrontTexture->GetRenderTexture(), pBackTexture->GetRenderTexture() });

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pRenderPass, info.unFrameIndex);

	info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_pDrawMeshMaterial);

	info.pRenderer->DrawMesh(info.pPrimaryCommand, &m_TransparentDrawMesh);

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MForwardTransparentWork::RenderDepthPeel(MRenderGraphNode* pGraphNode)
{
	MRenderInfo& info = *m_pRenderProgram->GetRenderInfo();

	if (nullptr == info.pViewport)
		return;

	if (info.vTransparentRenderGroup.empty())
		return;

	MRenderPass* pRenderPass = pGraphNode->GetRenderPass();
	if (!pRenderPass)
		return;

	MRenderGraphNodeOutput* pOutput0 = pGraphNode->GetOutput(0);
	if (!pOutput0)
		return;

	MRenderGraphTexture* pOutputTexture0 = pOutput0->GetRenderTexture();
	if (!pOutputTexture0)
		return;

	Vector2 v2OutputSize = pOutputTexture0->GetOutputSize();

	m_aFrameParamSet[0].UpdateShaderSharedParams(info);
	m_aFrameParamSet[1].UpdateShaderSharedParams(info);


	MViewport* pViewport = info.pViewport;

	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(0, 0, v2OutputSize.x, v2OutputSize.y));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));


	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pRenderPass, info.unFrameIndex);

	MRenderGraphTexture* pDepthTexture = pGraphNode->GetInput(1)->GetLinkedTexture();

	m_pDrawFillMaterial->GetTextureParams()->at(0)->pTexture = pDepthTexture->GetRenderTexture();
	m_pDrawFillMaterial->GetTextureParams()->at(0)->SetDirty();

	if (info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_pDrawFillMaterial))
	{
		info.pRenderer->DrawMesh(info.pPrimaryCommand, &m_TransparentDrawMesh);
	}

	for (uint32_t i = 1; i < pRenderPass->m_vSubpass.size(); ++i)
	{
		info.pRenderer->NextSubpass(info.pPrimaryCommand);

		for (MMaterialGroup& group : info.vTransparentRenderGroup)
		{
			MMaterial* pMaterial = group.m_pMaterial;
			//Ê¹ÓÃ²ÄÖÊ
			if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
				continue;

			info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, &m_aFrameParamSet[i % 2]);
			info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pMaterial->GetMaterialParamSet());

			for (MRenderableMeshComponent* pMeshComponent : group.m_vMeshComponents)
			{
				if (MSkeletonInstance* pSkeletonIns = pMeshComponent->GetSkeletonInstance())
				{
					info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pSkeletonIns->GetShaderParamSet());
				}
				info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pMeshComponent->GetShaderMeshParamSet());
				info.pRenderer->DrawMesh(info.pPrimaryCommand, pMeshComponent->GetMesh());
			}
		}
	}

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MForwardTransparentWork::InitializeMesh()
{
	MMesh<Vector2>& mesh = m_TransparentDrawMesh;
	mesh.ResizeVertices(4);
	Vector2* vVertices = (Vector2*)mesh.GetVertices();

	vVertices[0] = Vector2(-1, -1);
	vVertices[1] = Vector2(1, -1);
	vVertices[2] = Vector2(-1, 1);
	vVertices[3] = Vector2(1, 1);

	mesh.ResizeIndices(2, 3);
	uint32_t* vIndices = mesh.GetIndices();

	vIndices[0] = 0;
	vIndices[1] = 2;
	vIndices[2] = 1;

	vIndices[3] = 2;
	vIndices[4] = 3;
	vIndices[5] = 1;
}

void MForwardTransparentWork::ReleaseMesh()
{
	m_TransparentDrawMesh.DestroyBuffer(m_pEngine->GetDevice());
}

void MForwardTransparentWork::InitializeMaterial()
{
	MResource* pDPVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/depth_peel_blend.mvs");
	MResource* pDPBPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/depth_peel_blend.mps");
	MResource* pDPFPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/depth_peel_fill.mps");

	m_pDrawMeshMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
	m_pDrawMeshMaterial->SetMaterialType(MEMaterialType::ETransparentBlend);
	m_pDrawMeshMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawMeshMaterial->LoadPixelShader(pDPBPSResource);
	m_pDrawMeshMaterial->AddRef();

	m_pDrawFillMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
	m_pDrawFillMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	m_pDrawFillMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawFillMaterial->LoadPixelShader(pDPFPSResource);
	m_pDrawFillMaterial->AddRef();
}

void MForwardTransparentWork::ReleaseMaterial()
{
	m_pDrawMeshMaterial->SubRef();
	m_pDrawMeshMaterial = nullptr;

	m_pDrawFillMaterial->SubRef();
	m_pDrawFillMaterial = nullptr;
}

void MForwardTransparentWork::InitializeTexture()
{

	MTextureResource* pBlackTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>("Transparent_Black");
	MTextureResource* pWhiteTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>("Transparent_White");


	m_pBlackTexture = pBlackTextureRes->GetTextureTemplate();
	m_pBlackTexture->SetSize(Vector2(1, 1));
	m_pBlackTexture->FillColor(MColor(0, 0, 0, 0));
	m_pBlackTexture->GenerateBuffer(GetEngine()->GetDevice());

	m_pWhiteTexture = pBlackTextureRes->GetTextureTemplate();
	m_pWhiteTexture->SetSize(Vector2(1, 1));
	m_pWhiteTexture->FillColor(MColor(1, 1, 1, 1));
	m_pWhiteTexture->GenerateBuffer(GetEngine()->GetDevice());


	pBlackTextureRes->AddRef();
	pWhiteTextureRes->AddRef();
}

void MForwardTransparentWork::ReleaseTexture()
{
	m_pBlackTexture = nullptr;
	m_pWhiteTexture = nullptr;

	MTextureResource* pBlackTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>("Transparent_Black");
	MTextureResource* pWhiteTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>("Transparent_White");

	pBlackTextureRes->SubRef();
	pWhiteTextureRes->SubRef();
}

void MForwardTransparentWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardTransparentWork::InitializeRenderGraph failed. rg == nullptr");
		return;
	}

	Vector2 size(512, 512);

	MRenderGraphNode* pTransparentNode = pRenderGraph->AddRenderGraphNode("Transparent Node");
	if (pTransparentNode)
	{
		if (MRenderGraphNode* pForwardNode = pRenderGraph->FindRenderGraphNode("Forward Node"))
		{
			if (MRenderGraphNodeInput* pInput = pTransparentNode->AppendInput())
			{
				pInput->LinkTo(pForwardNode->GetOutput(0));
			}
			if (MRenderGraphNodeInput* pInput = pTransparentNode->AppendInput())
			{
				pInput->LinkTo(pForwardNode->GetOutput(1));
			}

		}

		if (MRenderGraphNodeOutput* pOutput = pTransparentNode->AppendOutput())
		{
			MRenderGraphTexture* pFrontTexture = pRenderGraph->AddRenderGraphTexture("Ts Front Tex");
			pFrontTexture->SetLayout(METextureLayout::ERGBA8);
			pFrontTexture->SetUsage(METextureUsage::ERenderBack);
			pFrontTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pFrontTexture->SetSize(Vector2(1.0f, 1.0f));

			pOutput->SetRenderTexture(pFrontTexture);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::Black_T);
		}
		if (MRenderGraphNodeOutput* pOutput = pTransparentNode->AppendOutput())
		{
			MRenderGraphTexture* pBackTexture = pRenderGraph->AddRenderGraphTexture("Ts Back Tex");
			pBackTexture->SetLayout(METextureLayout::ERGBA8);
			pBackTexture->SetUsage(METextureUsage::ERenderBack);
			pBackTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pBackTexture->SetSize(Vector2(1.0f, 1.0f));

			pOutput->SetRenderTexture(pBackTexture);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::Black_T);
		}
		if (MRenderGraphNodeOutput* pOutput = pTransparentNode->AppendOutput())
		{
			MRenderGraphTexture* pFrontDepthTexture0 = pRenderGraph->AddRenderGraphTexture("Ts Front Depth 0 Tex");
			pFrontDepthTexture0->SetLayout(METextureLayout::ER32);
			pFrontDepthTexture0->SetUsage(METextureUsage::ERenderBack);
			pFrontDepthTexture0->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pFrontDepthTexture0->SetSize(Vector2(1.0f, 1.0f));

			pOutput->SetRenderTexture(pFrontDepthTexture0);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::White);
		}
		if (MRenderGraphNodeOutput* pOutput = pTransparentNode->AppendOutput())
		{
			MRenderGraphTexture* pBackDepthTexture0 = pRenderGraph->AddRenderGraphTexture("Ts Back Depth 0 Tex");
			pBackDepthTexture0->SetLayout(METextureLayout::ER32);
			pBackDepthTexture0->SetUsage(METextureUsage::ERenderBack);
			pBackDepthTexture0->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pBackDepthTexture0->SetSize(Vector2(1.0f, 1.0f));

			pOutput->SetRenderTexture(pBackDepthTexture0);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::Black_T);
		}
		if (MRenderGraphNodeOutput* pOutput = pTransparentNode->AppendOutput())
		{
			MRenderGraphTexture* pFrontDepthTexture1 = pRenderGraph->AddRenderGraphTexture("Ts Front Depth 1 Tex");
			pFrontDepthTexture1->SetLayout(METextureLayout::ER32);
			pFrontDepthTexture1->SetUsage(METextureUsage::ERenderBack);
			pFrontDepthTexture1->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pFrontDepthTexture1->SetSize(Vector2(1.0f, 1.0f));

			pOutput->SetRenderTexture(pFrontDepthTexture1);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::White);
		}
		if (MRenderGraphNodeOutput* pOutput = pTransparentNode->AppendOutput())
		{
			MRenderGraphTexture* pBackDepthTexture1 = pRenderGraph->AddRenderGraphTexture("Ts Back Depth 1 Tex");
			pBackDepthTexture1->SetLayout(METextureLayout::ER32);
			pBackDepthTexture1->SetUsage(METextureUsage::ERenderBack);
			pBackDepthTexture1->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pBackDepthTexture1->SetSize(Vector2(1.0f, 1.0f));

			pOutput->SetRenderTexture(pBackDepthTexture1);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::Black_T);
		}

		SetupSubPass(*pTransparentNode->GetRenderPass());

		
		pTransparentNode->BindRenderFunction(std::bind(&MForwardTransparentWork::RenderDepthPeel, this, std::placeholders::_1));
		
	}

	if (MRenderGraphNode* pCombineNode = pRenderGraph->AddRenderGraphNode("Transparent Combine Node"))
	{
		if (MRenderGraphNode* pForwardNode = pRenderGraph->FindRenderGraphNode("Forward Node"))
		{
			if (MRenderGraphNodeInput* pInput = pCombineNode->AppendInput())
			{
				pInput->LinkTo(pForwardNode->GetOutput(0));
			}
			if (MRenderGraphNodeOutput* pOutput = pCombineNode->AppendOutput())
			{
				MRenderGraphTexture* pRenderTexture = pForwardNode->GetOutput(0)->GetRenderTexture();
				pOutput->SetRenderTexture(pRenderTexture);
				pOutput->SetClear(false);

				pRenderGraph->SetFinalOutput(pOutput);
			}
		}

		if (pTransparentNode)
		{
			if (MRenderGraphNodeInput* pInput = pCombineNode->AppendInput())
			{
				pInput->LinkTo(pTransparentNode->GetOutput(0));
			}
			if (MRenderGraphNodeInput* pInput = pCombineNode->AppendInput())
			{
				pInput->LinkTo(pTransparentNode->GetOutput(1));
			}
		}


		pCombineNode->BindRenderFunction(std::bind(&MForwardTransparentWork::Render, this, std::placeholders::_1));
	}



	BindTextureParam();
}

void MForwardTransparentWork::BindTextureParam()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();

	MRenderGraphTexture* pFrontTexture = pRenderGraph->FindRenderGraphTexture("Ts Front Tex");
	MRenderGraphTexture* pBackTexture = pRenderGraph->FindRenderGraphTexture("Ts Back Tex");
	MRenderGraphTexture* pFrontDepthTexture0 = pRenderGraph->FindRenderGraphTexture("Ts Front Depth 0 Tex");
	MRenderGraphTexture* pFrontDepthTexture1 = pRenderGraph->FindRenderGraphTexture("Ts Front Depth 1 Tex");
	MRenderGraphTexture* pBackDepthTexture0 = pRenderGraph->FindRenderGraphTexture("Ts Back Depth 0 Tex");
	MRenderGraphTexture* pBackDepthTexture1 = pRenderGraph->FindRenderGraphTexture("Ts Back Depth 1 Tex");

	m_aFrameParamSet[0].m_pTransparentFrontTextureParam->pTexture = pFrontDepthTexture1->GetRenderTexture();
	m_aFrameParamSet[0].m_pTransparentFrontTextureParam->SetDirty();
	m_aFrameParamSet[0].m_pTransparentBackTextureParam->pTexture = pBackDepthTexture1->GetRenderTexture();
	m_aFrameParamSet[0].m_pTransparentBackTextureParam->SetDirty();

	m_aFrameParamSet[1].m_pTransparentFrontTextureParam->pTexture = pFrontDepthTexture0->GetRenderTexture();
	m_aFrameParamSet[1].m_pTransparentFrontTextureParam->SetDirty();
	m_aFrameParamSet[1].m_pTransparentBackTextureParam->pTexture = pBackDepthTexture0->GetRenderTexture();
	m_aFrameParamSet[1].m_pTransparentBackTextureParam->SetDirty();


	std::vector<MShaderTextureParam*>& params = *m_pDrawMeshMaterial->GetTextureParams();
	params[0]->pTexture = pFrontTexture->GetRenderTexture();
	params[0]->SetDirty();
	params[1]->pTexture = pBackTexture->GetRenderTexture();
	params[1]->SetDirty();
}

void MForwardTransparentWork::SetupSubPass(MRenderPass& renderpass)
{
	static const uint32_t SUB_PASS_NUM = 6;

	renderpass.m_vSubpass.push_back(MSubpass());
	MSubpass& subpass = renderpass.m_vSubpass.back();

	/*
	* 0 output front
	* 1 output back
	* 2 input/output front
	* 3 input/output back
	* 4 input/output front depth
	* 5 input/output back depth
	*/

	subpass.m_vInputIndex = {};
	subpass.m_vOutputIndex = { 0, 1, 2, 3 };

	for (uint32_t i = 0; i < SUB_PASS_NUM; ++i)
	{
		renderpass.m_vSubpass.push_back(MSubpass());
		MSubpass& subpass = renderpass.m_vSubpass.back();

		if (i % 2)
		{
			subpass.m_vInputIndex = { 4, 5 };
			subpass.m_vOutputIndex = { 0, 1, 2, 3 };
		}
		else
		{
			subpass.m_vInputIndex = { 2, 3 };
			subpass.m_vOutputIndex = { 0, 1, 4, 5 };
		}
	}
}

void MForwardTransparentWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

