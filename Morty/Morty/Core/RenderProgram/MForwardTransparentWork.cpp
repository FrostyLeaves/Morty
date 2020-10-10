#include "MForwardTransparentWork.h"
#include "MTextureRenderTarget.h"
#include "MForwardRenderProgram.h"

#include "MEngine.h"
#include "MViewport.h"
#include "Model/MIMeshInstance.h"
#include "MResourceManager.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardTransparentWork, MObject)

MForwardTransparentWork::MForwardTransparentWork()
	: MObject()
	, m_TransparentDrawMesh(true)
	, m_pDrawMeshMaterial(nullptr)
	, m_pWhiteTexture(nullptr)
	, m_pBlackTexture(nullptr)
	, m_vTransparentFrontTexture()
	, m_vTransparentBackTexture()
	, m_pTransparentRenderTarget0(nullptr)
	, m_pTransparentRenderTarget1(nullptr)
	, m_pTransparentRenderTarget2(nullptr)
	, m_v2TransparentTextureSize(0.0f, 0.0f)
	, m_FrameParamSet()
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

	for (uint32_t i = 0; i < 3; ++i)
	{
		m_FrameParamSet[i].InitializeShaderParamSet(GetEngine());
	}
	InitializeRenderTargets();
	InitializeRenderPass();
}

void MForwardTransparentWork::Release()
{
	ReleaseRenderPass();
	ReleaseRenderTargets();
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_FrameParamSet[i].ReleaseShaderParamSet(GetEngine());
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

	for (uint32_t i = 0; i < 3; ++i)
		MForwardRenderProgram::UpdateShaderSharedParams(info, m_FrameParamSet[i]);

	UpdateTextureParams(info);

 	RenderToTarget(info, &m_TransWithClearRenderPass, m_pTransparentRenderTarget0, 0);
// 	for (uint32_t i = 0; i < 3; ++i)
// 	{
// 		RenderToTarget(info, &m_TransRenderPass, m_pTransparentRenderTarget1, 1);
// 		RenderToTarget(info, &m_TransRenderPass, m_pTransparentRenderTarget2, 2);
// 	}

	info.pRenderer->Wait();

	info.pRenderer->BeginRenderPass(&m_MeshRenderPass, info.pRenderTarget);

	std::vector<MShaderTextureParam*>& params = *m_pDrawMeshMaterial->GetTextureParams();

	params[0]->pTexture = m_vTransparentFrontTexture[info.unFrameIndex];
	params[0]->SetDirty();
	params[1]->pTexture = m_vTransparentBackTexture[info.unFrameIndex];
	params[1]->SetDirty();

	info.pRenderer->SetUseMaterial(m_pDrawMeshMaterial);

	info.pRenderer->DrawMesh(&m_TransparentDrawMesh);

	info.pRenderer->EndRenderPass();
}

void MForwardTransparentWork::RenderToTarget(MForwardRenderProgram::MRenderInfo& info, MRenderPass* pRenderPass, MTextureRenderTarget* pRenderTarget, const uint32_t& unFrameParamIdx)
{
	if (nullptr == info.pViewport)
		return;

	if (info.vTransparentRenderGroup.empty())
		return;

	MViewport* pViewport = info.pViewport;

	info.pRenderer->BeginRenderPass(pRenderPass, pRenderTarget);

	for (MMaterialGroup& group : info.vTransparentRenderGroup)
	{
		MMaterial* pMaterial = group.m_pMaterial;
		//Ê¹ÓÃ²ÄÖÊ
		if (!info.pRenderer->SetUseMaterial(pMaterial))
			continue;

		info.pRenderer->SetShaderParamSet(&m_FrameParamSet[unFrameParamIdx]);
		info.pRenderer->SetShaderParamSet(pMaterial->GetMaterialParamSet());
		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			info.pRenderer->SetShaderParamSet(pMeshIns->GetShaderMeshParamSet());
			info.pRenderer->DrawMesh(pMeshIns->GetMesh());
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
	MResourceManager* pManager = GetEngine()->GetResourceManager();
	m_pDrawMeshMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
	MResource* pDPVSResource = pManager->LoadResource("./Shader/depthPeeling.mvs");
	MResource* pDPPSResource = pManager->LoadResource("./Shader/depthPeeling.mps");

	m_pDrawMeshMaterial->SetMaterialType(MEMaterialType::EBlendTransparent);
	m_pDrawMeshMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawMeshMaterial->LoadPixelShader(pDPPSResource);

	m_pDrawMeshMaterial->AddRef();
}

void MForwardTransparentWork::ReleaseMaterial()
{
	m_pDrawMeshMaterial->SubRef();
	m_pDrawMeshMaterial = nullptr;
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
	m_pTransparentRenderTarget0 = GetEngine()->GetObjectManager()->CreateObject<MTextureRenderTarget>();
	m_pTransparentRenderTarget1 = GetEngine()->GetObjectManager()->CreateObject<MTextureRenderTarget>();
	m_pTransparentRenderTarget2 = GetEngine()->GetObjectManager()->CreateObject<MTextureRenderTarget>();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_vTransparentFrontTexture[i] = new MRenderBackTexture();
		m_vTransparentFrontTexture[i]->SetType(METextureLayout::ERGBA8);

		m_vTransparentBackTexture[i] = new MRenderBackTexture();
		m_vTransparentBackTexture[i]->SetType(METextureLayout::ERGBA8);

		vBackTexture0[i] = new MRenderBackTexture();
		vBackTexture0[i]->SetType(METextureLayout::ER32);

		vBackTexture1[i] = new MRenderBackTexture();
		vBackTexture1[i]->SetType(METextureLayout::ER32);

		vBackTexture2[i] = new MRenderBackTexture();
		vBackTexture2[i]->SetType(METextureLayout::ER32);

		vBackTexture3[i] = new MRenderBackTexture();
		vBackTexture3[i]->SetType(METextureLayout::ER32);

		m_vRenderTargetTexture.push_back(m_vTransparentFrontTexture[i]);
		m_vRenderTargetTexture.push_back(m_vTransparentBackTexture[i]);
		m_vRenderTargetTexture.push_back(vBackTexture0[i]);
		m_vRenderTargetTexture.push_back(vBackTexture1[i]);
		m_vRenderTargetTexture.push_back(vBackTexture2[i]);
		m_vRenderTargetTexture.push_back(vBackTexture3[i]);
	}


	m_pTransparentRenderTarget0->SetBackTexture(m_vTransparentFrontTexture, 0);
	m_pTransparentRenderTarget0->SetBackTexture(m_vTransparentBackTexture, 1);
	m_pTransparentRenderTarget0->SetBackTexture(vBackTexture2, 2);
	m_pTransparentRenderTarget0->SetBackTexture(vBackTexture3, 3);


	m_pTransparentRenderTarget1->SetBackTexture(m_vTransparentFrontTexture, 0);
	m_pTransparentRenderTarget1->SetBackTexture(m_vTransparentBackTexture, 1);
	m_pTransparentRenderTarget1->SetBackTexture(vBackTexture0, 2);
	m_pTransparentRenderTarget1->SetBackTexture(vBackTexture1, 3);

	m_pTransparentRenderTarget2->SetBackTexture(m_vTransparentFrontTexture, 0);
	m_pTransparentRenderTarget2->SetBackTexture(m_vTransparentBackTexture, 1);
	m_pTransparentRenderTarget2->SetBackTexture(vBackTexture2, 2);
	m_pTransparentRenderTarget2->SetBackTexture(vBackTexture3, 3);

}

void MForwardTransparentWork::ReleaseRenderTargets()
{

	if (m_pTransparentRenderTarget0)
	{
		GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget0);
		m_pTransparentRenderTarget0->DeleteLater();
		m_pTransparentRenderTarget0 = nullptr;
	}

	if (m_pTransparentRenderTarget1)
	{
		GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget1);
		m_pTransparentRenderTarget1->DeleteLater();
		m_pTransparentRenderTarget1 = nullptr;
	}
	if (m_pTransparentRenderTarget2)
	{
		GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget2);
		m_pTransparentRenderTarget2->DeleteLater();
		m_pTransparentRenderTarget2 = nullptr;
	}

	for (MRenderBackTexture* pBackTexture : m_vRenderTargetTexture)
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

	m_TransRenderPass.m_vBackDesc.push_back(descNoClear);
	m_TransRenderPass.m_vBackDesc.push_back(descNoClear);
	m_TransRenderPass.m_vBackDesc.push_back(descWhite);
	m_TransRenderPass.m_vBackDesc.push_back(descBlackT);

	GetEngine()->GetDevice()->GenerateRenderPass(&m_TransWithClearRenderPass, m_pTransparentRenderTarget0);
	GetEngine()->GetDevice()->GenerateRenderPass(&m_TransRenderPass, m_pTransparentRenderTarget1);

	MRenderPass::MTargetDesc descMesh;
	descMesh.bClearWhenRender = false;
	m_MeshRenderPass.m_vBackDesc.push_back(descMesh);
	m_MeshRenderPass.m_DepthDesc.bClearWhenRender = false;
	GetEngine()->GetDevice()->GenerateRenderPass(&m_MeshRenderPass, pRenderTarget);
}

void MForwardTransparentWork::ReleaseRenderPass()
{
	GetEngine()->GetDevice()->DestroyRenderPass(&m_TransWithClearRenderPass);
	GetEngine()->GetDevice()->DestroyRenderPass(&m_TransRenderPass);
	GetEngine()->GetDevice()->DestroyRenderPass(&m_MeshRenderPass);
}

void MForwardTransparentWork::CheckTransparentTextureSize(MForwardRenderProgram::MRenderInfo& info)
{
	Vector2 v2Size = info.pViewport->GetSize();

	if (m_v2TransparentTextureSize == v2Size)
		return;

	m_v2TransparentTextureSize = v2Size;

	MIDevice* pDevice = GetEngine()->GetDevice();

	for (MRenderBackTexture* pTexture : m_vRenderTargetTexture)
	{
		if (pTexture)
		{
			pTexture->DestroyBuffer(pDevice);
			pTexture->SetSize(v2Size);
			pTexture->GenerateBuffer(pDevice);
		}
	}

	m_pTransparentRenderTarget0->Resize(m_v2TransparentTextureSize);
	m_pTransparentRenderTarget1->Resize(m_v2TransparentTextureSize);
	m_pTransparentRenderTarget2->Resize(m_v2TransparentTextureSize);
}

void MForwardTransparentWork::UpdateTextureParams(MForwardRenderProgram::MRenderInfo& info)
{
	uint32_t unFrameIdx = info.unFrameIndex;

	m_FrameParamSet[0].m_pTransparentFrontTextureParam->pTexture = m_pBlackTexture;
	m_FrameParamSet[0].m_pTransparentBackTextureParam->pTexture = m_pWhiteTexture;
	m_FrameParamSet[0].m_pTransparentFrontTextureParam->SetDirty();
	m_FrameParamSet[0].m_pTransparentBackTextureParam->SetDirty();

	m_FrameParamSet[1].m_pTransparentFrontTextureParam->pTexture = vBackTexture2[unFrameIdx];
	m_FrameParamSet[1].m_pTransparentBackTextureParam->pTexture = vBackTexture3[unFrameIdx];
	m_FrameParamSet[1].m_pTransparentFrontTextureParam->SetDirty();
	m_FrameParamSet[1].m_pTransparentBackTextureParam->SetDirty();

	m_FrameParamSet[2].m_pTransparentFrontTextureParam->pTexture = vBackTexture0[unFrameIdx];
	m_FrameParamSet[2].m_pTransparentBackTextureParam->pTexture = vBackTexture1[unFrameIdx];
	m_FrameParamSet[2].m_pTransparentFrontTextureParam->SetDirty();
	m_FrameParamSet[2].m_pTransparentBackTextureParam->SetDirty();
}

void MForwardTransparentWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

