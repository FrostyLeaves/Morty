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

void MForwardTransparentWork::DrawTransparentMesh(MForwardRenderProgram::MRenderInfo& info)
{
	if (info.vTransparentRenderGroup.empty())
		return;

	CheckTransparentTextureSize(info);

	uint32_t unFrameIdx = info.pRenderer->GetFrameIndex();

	for (uint32_t i = 0; i < 3; ++i)
		MForwardRenderProgram::UpdateShaderSharedParams(info, m_FrameParamSet[i]);

	m_FrameParamSet[0].m_pTransparentFrontTextureParam->pTexture = m_pBlackTexture;
	m_FrameParamSet[0].m_pTransparentBackTextureParam->pTexture = m_pWhiteTexture;

	m_FrameParamSet[1].m_pTransparentFrontTextureParam->pTexture = vBackTexture2[unFrameIdx];
	m_FrameParamSet[1].m_pTransparentBackTextureParam->pTexture = vBackTexture3[unFrameIdx];

	m_FrameParamSet[2].m_pTransparentFrontTextureParam->pTexture = vBackTexture0[unFrameIdx];
	m_FrameParamSet[2].m_pTransparentBackTextureParam->pTexture = vBackTexture1[unFrameIdx];

	
	RenderToTarget(info, m_pTransparentRenderTarget0, 0);

	for (uint32_t i = 0; i < 3; ++i)
	{
		RenderToTarget(info, m_pTransparentRenderTarget1, 1);
		RenderToTarget(info, m_pTransparentRenderTarget2, 2);
	}


	info.pRenderer->BeginRenderPass(info.pRenderTarget);

	MMaterialResource* pTextureMaterial = GetEngine()->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DEPTH_PEELING);
	std::vector<MShaderTextureParam*>& params = *pTextureMaterial->GetTextureParams();

	params[0]->pTexture = m_vTransparentFrontTexture[info.unFrameIndex];
	params[1]->pTexture = m_vTransparentBackTexture[info.unFrameIndex];

	info.pRenderer->SetUseMaterial(pTextureMaterial);

	info.pRenderer->DrawMesh(&m_TransparentDrawMesh);

	info.pRenderer->EndRenderPass(info.pRenderTarget);
}

void MForwardTransparentWork::RenderToTarget(MForwardRenderProgram::MRenderInfo& info, MTextureRenderTarget* pRenderTarget, const uint32_t& unFrameParamIdx)
{
	if (nullptr == info.pViewport)
		return;

	if (info.vTransparentRenderGroup.empty())
		return;

	MViewport* pViewport = info.pViewport;

	info.pRenderer->BeginRenderPass(pRenderTarget);

	//Set Current DepthTexture.
// 	std::array<MRenderDepthTexture*, M_BUFFER_NUM> vDepthTextures;
// 	vDepthTextures.fill(info.pRenderTarget->GetCurrDepthTexture());
// 	pRenderTarget->SetDepthTexture(vDepthTextures);


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
	

	info.pRenderer->EndRenderPass(pRenderTarget);
}

void MForwardTransparentWork::OnCreated()
{
	InitializeMesh();
	InitializeTexture();

	for (uint32_t i = 0; i < 3; ++i)
	{
		m_FrameParamSet[i].InitializeShaderParamSet(GetEngine());
	}
	InitializeRenderTargets();
}

void MForwardTransparentWork::OnDelete()
{
	ReleaseRenderTargets();
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_FrameParamSet[i].ReleaseShaderParamSet(GetEngine());
	}
	ReleaseTexture();
	ReleaseMesh();
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

void MForwardTransparentWork::InitializeTexture()
{

	MTextureResource* pBlackTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_BLACK);
	MTextureResource* pWhiteTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_WHITE);

	m_pBlackTexture = pBlackTextureRes->GetTextureTemplate();
	m_pWhiteTexture = pWhiteTextureRes->GetTextureTemplate();
}

void MForwardTransparentWork::ReleaseTexture()
{
	m_pBlackTexture = nullptr;
	m_pWhiteTexture = nullptr;
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


	m_pTransparentRenderTarget0->SetBackTexture(m_vTransparentFrontTexture, 0, MColor::Black_T);
	m_pTransparentRenderTarget0->SetBackTexture(m_vTransparentFrontTexture, 1, MColor::Black_T);
	m_pTransparentRenderTarget0->SetBackTexture(vBackTexture2, 2, MColor::White);
	m_pTransparentRenderTarget0->SetBackTexture(vBackTexture3, 3, MColor::Black_T);
//	m_pTransparentRenderTarget0->SetPrevLayerFrontDepthTexture(m_pBlackTexture);
//	m_pTransparentRenderTarget0->SetPrevLayerBackDepthTexture(m_pWhiteTexture);
	m_pTransparentRenderTarget0->SetBackTexture(vBackTexture3, 3, MColor::Black_T);


	m_pTransparentRenderTarget1->SetBackTexture(m_vTransparentFrontTexture, 0);
	m_pTransparentRenderTarget1->SetBackTexture(m_vTransparentBackTexture, 1);
	m_pTransparentRenderTarget1->SetBackTexture(vBackTexture0, 2, MColor::White);
	m_pTransparentRenderTarget1->SetBackTexture(vBackTexture1, 3, MColor::Black_T);
	//m_pTransparentRenderTarget1->SetPrevLayerFrontDepthTexture(vBackTexture2);
	//m_pTransparentRenderTarget1->SetPrevLayerBackDepthTexture(vBackTexture3);

	m_pTransparentRenderTarget2->SetBackTexture(m_vTransparentFrontTexture, 0);
	m_pTransparentRenderTarget2->SetBackTexture(m_vTransparentBackTexture, 1);
	m_pTransparentRenderTarget2->SetBackTexture(vBackTexture2, 2, MColor::White);
	m_pTransparentRenderTarget2->SetBackTexture(vBackTexture3, 3, MColor::Black_T);
	//m_pTransparentRenderTarget2->SetPrevLayerFrontDepthTexture(vBackTexture0);
	//m_pTransparentRenderTarget2->SetPrevLayerBackDepthTexture(vBackTexture1);


	MRenderPass::MRTDesc desc0, desc1, desc2, desc3;

	desc0.bClearWhenRender = false;
	desc1.bClearWhenRender = false;

	m_pTransparentRenderTarget0->m_RenderPass.m_vBackDesc.push_back(desc0);
	m_pTransparentRenderTarget0->m_RenderPass.m_vBackDesc.push_back(desc1);
	m_pTransparentRenderTarget0->m_RenderPass.m_vBackDesc.push_back(desc2);
	m_pTransparentRenderTarget0->m_RenderPass.m_vBackDesc.push_back(desc3);

	m_pTransparentRenderTarget1->m_RenderPass.m_vBackDesc.push_back(desc0);
	m_pTransparentRenderTarget1->m_RenderPass.m_vBackDesc.push_back(desc1);
	m_pTransparentRenderTarget1->m_RenderPass.m_vBackDesc.push_back(desc2);
	m_pTransparentRenderTarget1->m_RenderPass.m_vBackDesc.push_back(desc3);

	m_pTransparentRenderTarget2->m_RenderPass.m_vBackDesc.push_back(desc0);
	m_pTransparentRenderTarget2->m_RenderPass.m_vBackDesc.push_back(desc1);
	m_pTransparentRenderTarget2->m_RenderPass.m_vBackDesc.push_back(desc2);
	m_pTransparentRenderTarget2->m_RenderPass.m_vBackDesc.push_back(desc3);

}

void MForwardTransparentWork::ReleaseRenderTargets()
{
	if (m_pTransparentRenderTarget0)
	{
		m_pTransparentRenderTarget0->DeleteLater();
		m_pTransparentRenderTarget0 = nullptr;
	}

	if (m_pTransparentRenderTarget1)
	{
		m_pTransparentRenderTarget1->DeleteLater();
		m_pTransparentRenderTarget1 = nullptr;
	}
	if (m_pTransparentRenderTarget2)
	{
		m_pTransparentRenderTarget2->DeleteLater();
		m_pTransparentRenderTarget2 = nullptr;
	}

	for (MRenderBackTexture* pBackTexture : m_vRenderTargetTexture)
	{
		pBackTexture->DestroyBuffer(GetEngine()->GetDevice());
		delete pBackTexture;
	}
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


	GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget0);
	GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget1);
	GetEngine()->GetDevice()->DestroyRenderTarget(m_pTransparentRenderTarget2);


	GetEngine()->GetDevice()->GenerateRenderTarget(m_pTransparentRenderTarget0, m_v2TransparentTextureSize.x, m_v2TransparentTextureSize.y);
	GetEngine()->GetDevice()->GenerateRenderTarget(m_pTransparentRenderTarget1, m_v2TransparentTextureSize.x, m_v2TransparentTextureSize.y);
	GetEngine()->GetDevice()->GenerateRenderTarget(m_pTransparentRenderTarget2, m_v2TransparentTextureSize.x, m_v2TransparentTextureSize.y);
}

