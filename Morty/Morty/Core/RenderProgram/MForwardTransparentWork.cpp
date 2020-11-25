#include "MForwardTransparentWork.h"
#include "MTextureRenderTarget.h"
#include "MForwardRenderProgram.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MSkeleton.h"
#include "Model/MIMeshInstance.h"
#include "MResourceManager.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardTransparentWork, MObject)

MForwardTransparentWork::MForwardTransparentWork()
	: MObject()
	, m_TransparentDrawMesh(true)
	, m_pDrawMeshMaterial(nullptr)
	, m_pDrawFillMaterial(nullptr)
	, m_pWhiteTexture(nullptr)
	, m_pBlackTexture(nullptr)
	, m_pTransparentRenderTarget(nullptr)
	, m_v2TransparentTextureSize(0.0f, 0.0f)
	, m_aFrameParamSet()
	, vBackTexture()
	, vFrontTexture()
	, vBackDepthTexture()
	, vFrontDepthTexture()
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
	InitializeRenderTargets();
	InitializeRenderPass();
}

void MForwardTransparentWork::Release()
{
	ReleaseRenderPass();
	ReleaseRenderTargets();
	for (uint32_t i = 0; i < 2; ++i)
	{
		m_aFrameParamSet[i].ReleaseShaderParamSet(GetEngine());
	}

	ReleaseTexture();
	ReleaseMesh();
	ReleaseMaterial();
}

void MForwardTransparentWork::DrawTransparentMesh(MForwardRenderProgram::MRenderInfo& info)
{
	if (info.vTransparentRenderGroup.empty())
		return;

	CheckTransparentTextureSize(info);

	uint32_t unFrameIdx = info.pRenderer->GetFrameIndex();

	UpdateShaderSharedParams(info);

 	RenderToTarget(info, &m_TransWithClearRenderPass, m_pTransparentRenderTarget, 1);

	info.pRenderer->BeginRenderPass(&m_MeshRenderPass, info.pRenderTarget);

	std::vector<MShaderTextureParam*>& params = *m_pDrawMeshMaterial->GetTextureParams();

	params[0]->pTexture = vFrontTexture[info.unFrameIndex];
	params[0]->SetDirty();
	params[1]->pTexture = vBackTexture[info.unFrameIndex];
	params[1]->SetDirty();

	info.pRenderer->SetUseMaterial(m_pDrawMeshMaterial);

	info.pRenderer->DrawMesh(&m_TransparentDrawMesh);

	info.pRenderer->EndRenderPass();
}

void MForwardTransparentWork::RenderToTarget(MForwardRenderProgram::MRenderInfo& info, MRenderPass* pRenderPass, MTextureRenderTarget* pRenderTarget, const uint32_t& unTargetIdx)
{
	if (nullptr == info.pViewport)
		return;

	if (info.vTransparentRenderGroup.empty())
		return;

	MViewport* pViewport = info.pViewport;

	info.pRenderer->BeginRenderPass(pRenderPass, pRenderTarget);

	m_pDrawFillMaterial->GetTextureParams()->at(0)->pTexture = info.pRenderTarget->GetCurrDepthTexture();
	m_pDrawFillMaterial->GetTextureParams()->at(0)->SetDirty();

 	if (info.pRenderer->SetUseMaterial(m_pDrawFillMaterial))
 	{
 		info.pRenderer->DrawMesh(&m_TransparentDrawMesh);
 	}

	for (uint32_t i = 1; i < pRenderPass->m_vSubpass.size(); ++i)
	{
		info.pRenderer->NextSubpass();

		for (MMaterialGroup& group : info.vTransparentRenderGroup)
		{
			MMaterial* pMaterial = group.m_pMaterial;
			//Ê¹ÓÃ²ÄÖÊ
			if (!info.pRenderer->SetUseMaterial(pMaterial))
				continue;

			info.pRenderer->SetShaderParamSet(&m_aFrameParamSet[i % 2]);
			info.pRenderer->SetShaderParamSet(pMaterial->GetMaterialParamSet());

			for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
			{
				if (MSkeletonInstance* pSkeletonIns = pMeshIns->GetSkeletonInstance())
				{
					info.pRenderer->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
				}
				info.pRenderer->SetShaderParamSet(pMeshIns->GetShaderMeshParamSet());
				info.pRenderer->DrawMesh(pMeshIns->GetMesh());
			}
		}
	}
	
	info.pRenderer->EndRenderPass();
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

void MForwardTransparentWork::InitializeRenderTargets()
{
	m_pTransparentRenderTarget = GetEngine()->GetObjectManager()->CreateObject<MTextureRenderTarget>();
	
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		vFrontTexture[i] = new MRenderSubpassTexture();
		vFrontTexture[i]->SetType(METextureLayout::ERGBA8);

		vBackTexture[i] = new MRenderSubpassTexture();
		vBackTexture[i]->SetType(METextureLayout::ERGBA8);

		m_vRenderTargetTexture.push_back(vFrontTexture[i]);
		m_vRenderTargetTexture.push_back(vBackTexture[i]);

		for (uint32_t n = 0; n < 2; ++n)
		{
			vFrontDepthTexture[n][i] = new MRenderSubpassTexture();
			vFrontDepthTexture[n][i]->SetType(METextureLayout::ER32);

			vBackDepthTexture[n][i] = new MRenderSubpassTexture();
			vBackDepthTexture[n][i]->SetType(METextureLayout::ER32);

			m_vRenderTargetTexture.push_back(vFrontDepthTexture[n][i]);
			m_vRenderTargetTexture.push_back(vBackDepthTexture[n][i]);
		}

	}

	m_pTransparentRenderTarget->SetBackTexture(vFrontTexture, 0);
	m_pTransparentRenderTarget->SetBackTexture(vBackTexture, 1);
	m_pTransparentRenderTarget->SetBackTexture(vFrontDepthTexture[0],2);
	m_pTransparentRenderTarget->SetBackTexture(vBackDepthTexture[0], 3);
	m_pTransparentRenderTarget->SetBackTexture(vFrontDepthTexture[1], 4);
	m_pTransparentRenderTarget->SetBackTexture(vBackDepthTexture[1], 5);
	

}

void MForwardTransparentWork::ReleaseRenderTargets()
{

	if (m_pTransparentRenderTarget)
	{
		GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget);
		m_pTransparentRenderTarget->DeleteLater();
		m_pTransparentRenderTarget = nullptr;
	}

	for (MIRenderBackTexture* pBackTexture : m_vRenderTargetTexture)
	{
		pBackTexture->DestroyBuffer(GetEngine()->GetDevice());
		delete pBackTexture;
	}
}

void MForwardTransparentWork::InitializeRenderPass()
{
	if (!m_pRenderProgram)
	{
		MLogManager::GetInstance()->Error("MForwardTransparentWork::InitializeRenderPass error, rp == nullptr");
		return;
	}

	MIRenderTarget* pRenderTarget = m_pRenderProgram->GetRenderTarget();
	if (!pRenderTarget)
	{
		MLogManager::GetInstance()->Error("MForwardTransparentWork::InitializeRenderPass error, rt == nullptr");
		return;
	}

	MRenderPass::MTargetDesc descNoClear(false, MColor::Black_T);
	MRenderPass::MTargetDesc descWhite(true, MColor::White);
	MRenderPass::MTargetDesc descBlackT(true, MColor::Black_T);

	m_TransWithClearRenderPass.m_vBackDesc.push_back(descBlackT);
	m_TransWithClearRenderPass.m_vBackDesc.push_back(descBlackT);
	m_TransWithClearRenderPass.m_vBackDesc.push_back(descWhite);
	m_TransWithClearRenderPass.m_vBackDesc.push_back(descBlackT);
	m_TransWithClearRenderPass.m_vBackDesc.push_back(descWhite);
	m_TransWithClearRenderPass.m_vBackDesc.push_back(descBlackT);

	SetupSubPass(m_TransWithClearRenderPass);

	GetEngine()->GetDevice()->GenerateRenderPass(&m_TransWithClearRenderPass, m_pTransparentRenderTarget);

	MRenderPass::MTargetDesc descMesh;
	descMesh.bClearWhenRender = false;
	m_MeshRenderPass.m_vBackDesc.push_back(descMesh);
	m_MeshRenderPass.m_DepthDesc.bClearWhenRender = false;
	GetEngine()->GetDevice()->GenerateRenderPass(&m_MeshRenderPass, pRenderTarget);
}

void MForwardTransparentWork::ReleaseRenderPass()
{
	GetEngine()->GetDevice()->DestroyRenderPass(&m_TransWithClearRenderPass);
	GetEngine()->GetDevice()->DestroyRenderPass(&m_MeshRenderPass);
}

void MForwardTransparentWork::CheckTransparentTextureSize(MForwardRenderProgram::MRenderInfo& info)
{
	Vector2 v2Size = info.pViewport->GetSize();

	if (m_v2TransparentTextureSize == v2Size)
		return;

	m_v2TransparentTextureSize = v2Size;

	MIDevice* pDevice = GetEngine()->GetDevice();

	for (MIRenderBackTexture* pTexture : m_vRenderTargetTexture)
	{
		if (pTexture)
		{
			pTexture->DestroyBuffer(pDevice);
			pTexture->SetSize(v2Size);
			pTexture->GenerateBuffer(pDevice);
		}
	}

	m_pTransparentRenderTarget->Resize(m_v2TransparentTextureSize);
}

void MForwardTransparentWork::UpdateShaderSharedParams(MForwardRenderProgram::MRenderInfo& info)
{
	uint32_t unFrameIdx = info.unFrameIndex;

	MForwardRenderProgram::UpdateShaderSharedParams(info, m_aFrameParamSet[0]);
	m_aFrameParamSet[0].m_pTransparentFrontTextureParam->pTexture = vFrontDepthTexture[1][unFrameIdx];
	m_aFrameParamSet[0].m_pTransparentBackTextureParam->pTexture = vBackDepthTexture[1][unFrameIdx];
	m_aFrameParamSet[0].m_pTransparentFrontTextureParam->SetDirty();
	m_aFrameParamSet[0].m_pTransparentBackTextureParam->SetDirty();

	MForwardRenderProgram::UpdateShaderSharedParams(info, m_aFrameParamSet[1]);
	m_aFrameParamSet[1].m_pTransparentFrontTextureParam->pTexture = vFrontDepthTexture[0][unFrameIdx];
	m_aFrameParamSet[1].m_pTransparentBackTextureParam->pTexture = vBackDepthTexture[0][unFrameIdx];
	m_aFrameParamSet[1].m_pTransparentFrontTextureParam->SetDirty();
	m_aFrameParamSet[1].m_pTransparentBackTextureParam->SetDirty();
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

