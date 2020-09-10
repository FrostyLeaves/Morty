#include "MForwardTransparentWork.h"
#include "MTransparentRenderTarget.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardTransparentWork, MObject)

MForwardTransparentWork::MForwardTransparentWork()
	: MObject()
	, m_TransparentDrawMesh(true)
	, m_vTransparentFrontTexture()
	, m_vTransparentBackTexture()
	, m_pTransparentRenderTarget0(nullptr)
	, m_pTransparentRenderTarget1(nullptr)
	, m_pTransparentRenderTarget2(nullptr)
	, m_v2TransparentTextureSize(0.0f, 0.0f)
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

	m_pTransparentRenderTarget0->Render(info.pRenderer, info.pViewport, info.pRenderTarget, &info.vTransparentRenderGroup);

	for (uint32_t i = 0; i < 3; ++i)
	{
		m_pTransparentRenderTarget1->Render(info.pRenderer, info.pViewport, info.pRenderTarget, &info.vTransparentRenderGroup);
		m_pTransparentRenderTarget2->Render(info.pRenderer, info.pViewport, info.pRenderTarget, &info.vTransparentRenderGroup);
	}

	MMaterialResource* pTextureMaterial = GetEngine()->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DEPTH_PEELING);
	std::vector<MShaderTextureParam*>& params = *pTextureMaterial->GetTextureParams();

	params[0]->pTexture = m_vTransparentFrontTexture[info.unFrameIndex];
	params[1]->pTexture = m_vTransparentBackTexture[info.unFrameIndex];
	info.pRenderer->SetUseMaterial(pTextureMaterial);
	info.pRenderer->DrawMesh(&m_TransparentDrawMesh);
}

void MForwardTransparentWork::OnCreated()
{
	InitializeMesh();
	InitializeRenderTargets();
}

void MForwardTransparentWork::OnDelete()
{
	ReleaseMesh();
	ReleaseRenderTargets();
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

void MForwardTransparentWork::InitializeRenderTargets()
{
	m_pTransparentRenderTarget0 = GetEngine()->GetObjectManager()->CreateObject<MTransparentRenderTarget>();
	m_pTransparentRenderTarget1 = GetEngine()->GetObjectManager()->CreateObject<MTransparentRenderTarget>();
	m_pTransparentRenderTarget2 = GetEngine()->GetObjectManager()->CreateObject<MTransparentRenderTarget>();

	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture0;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture1;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture2;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture3;
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
	m_pTransparentRenderTarget0->ResetPrevLayerTexture();

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
}

