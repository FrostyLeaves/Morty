#include "MTransparentRenderWork.h"
#include "MForwardRenderProgram.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MSkeleton.h"
#include "MRenderCommand.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MTextureResource.h"
#include "MMaterialResource.h"

#include "MRenderableMeshComponent.h"

MORTY_CLASS_IMPLEMENT(MTransparentRenderWork, MObject)

MTransparentRenderWork::MTransparentRenderWork()
	: MObject()
	, m_pWhiteTexture(nullptr)
	, m_pBlackTexture(nullptr)
	, m_pDrawFillMaterial(nullptr)
	, m_pDrawPeelMaterial(nullptr)
	, m_pOutputTexture(nullptr)
	, m_pDepthTexture(nullptr)
	, m_TransparentDrawMesh(true)
	, m_peelRenderPass()
	, m_fillRenderPass()
	, m_aFrameParamSet()
{
	
}

MTransparentRenderWork::~MTransparentRenderWork()
{
}

void MTransparentRenderWork::OnCreated()
{
	InitializeMesh();
	InitializeTexture();
	InitializeMaterial();
	InitializeFrameShaderParams();


	InitializePeelRenderPass();
	InitializeFillRenderPass();
}

void MTransparentRenderWork::OnDelete()
{
	ReleaseFillRenderPass();
	ReleasePeelRenderPass();

	ReleaseFrameShaderParams();
	ReleaseMaterial();
	ReleaseTexture();
	ReleaseMesh();

	Super::OnDelete();
}

void MTransparentRenderWork::SetRenderTarget(MTexture* pOutputTexture, MTexture* pDepthTexture)
{
	if (m_pOutputTexture = pOutputTexture)
	{
		Resize(m_pOutputTexture->GetSize());
	}

	m_pDepthTexture = pDepthTexture;
}

void MTransparentRenderWork::Render(MRenderInfo& info)
{
	if (info.m_tTransparentGroupMesh.empty())
		return;
	
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
		return;

	pCommand->SetRenderToTextureBarrier({ m_pFrontTexture, m_pBackTexture });

	pCommand->BeginRenderPass(&m_fillRenderPass);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));

	pCommand->SetUseMaterial(m_pDrawFillMaterial);

	pCommand->DrawMesh(&m_TransparentDrawMesh);

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::RenderDepthPeel(MRenderInfo& info)
{
	if (nullptr == info.pViewport)
		return;

	if (info.m_tTransparentGroupMesh.empty())
		return;

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
		return;

	m_aFrameParamSet[0].UpdateShaderSharedParams(info);
	m_aFrameParamSet[1].UpdateShaderSharedParams(info);


	pCommand->SetRenderToTextureBarrier({ m_pDepthTexture });

	pCommand->BeginRenderPass(&m_peelRenderPass);


	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	Vector2 v2Size = info.pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	if (m_pDrawPeelMaterial->GetTextureParams()->at(0)->pTexture != m_pDepthTexture)
	{
		m_pDrawPeelMaterial->GetTextureParams()->at(0)->pTexture = m_pDepthTexture;
		m_pDrawPeelMaterial->GetTextureParams()->at(0)->SetDirty();
	}
	

	if (pCommand->SetUseMaterial(m_pDrawPeelMaterial))
	{
		pCommand->DrawMesh(&m_TransparentDrawMesh);
	}

	for (uint32_t i = 1; i < m_peelRenderPass.m_vSubpass.size(); ++i)
	{
		pCommand->NextSubpass();

		for (auto& pr : info.m_tTransparentGroupMesh)
		{
			std::shared_ptr<MMaterial> pMaterial = pr.first;
			//ʹ�ò���
			if (!pCommand->SetUseMaterial(pMaterial))
				continue;

			pCommand->SetShaderParamSet(&m_aFrameParamSet[i % 2]);

			for (MRenderableMeshComponent* pMeshComponent : pr.second)
			{
				if (std::shared_ptr<MSkeletonInstance> pSkeletonIns = pMeshComponent->GetSkeletonInstance())
				{
					pCommand->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
				}
				pCommand->SetShaderParamSet(pMeshComponent->GetShaderMeshParamSet());
				pCommand->DrawMesh(pMeshComponent->GetMesh());
			}
		}
	}

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::Resize(const Vector2& v2Size)
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_fillRenderPass.Resize(pRenderSystem->GetDevice());

	pRenderSystem->ResizeFrameBuffer(m_peelRenderPass, v2Size);
}

void MTransparentRenderWork::InitializeMesh()
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

void MTransparentRenderWork::ReleaseMesh()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_TransparentDrawMesh.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> pDPVSResource = pResourceSystem->LoadResource("./Shader/depth_peel_blend.mvs");
	std::shared_ptr<MResource> pDPBPSResource = pResourceSystem->LoadResource("./Shader/depth_peel_blend.mps");
	std::shared_ptr<MResource> pDPFPSResource = pResourceSystem->LoadResource("./Shader/depth_peel_fill.mps");

	m_pDrawPeelMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pDrawPeelMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	m_pDrawPeelMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawPeelMaterial->LoadPixelShader(pDPFPSResource);
	m_pDrawPeelMaterial->AddRef();


	m_pDrawFillMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pDrawFillMaterial->SetMaterialType(MEMaterialType::ETransparentBlend);
	m_pDrawFillMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawFillMaterial->LoadPixelShader(pDPBPSResource);
	m_pDrawFillMaterial->AddRef();

	std::vector<MShaderTextureParam*>& params = *m_pDrawFillMaterial->GetTextureParams();
	params[0]->pTexture = m_pFrontTexture;
	params[0]->SetDirty();
	params[1]->pTexture = m_pBackTexture;
	params[1]->SetDirty();
}

void MTransparentRenderWork::ReleaseMaterial()
{
	m_pDrawFillMaterial->SubRef();
	m_pDrawFillMaterial = nullptr;

	m_pDrawPeelMaterial->SubRef();
	m_pDrawPeelMaterial = nullptr;
}

void MTransparentRenderWork::InitializeTexture()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	m_pBlackTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_Black");
	m_pWhiteTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_White");

	static MByte black[4] = { 0, 0 ,0 ,0 };
	static MByte white[4] = { 255, 255 ,255 ,255 };
	m_pBlackTexture->LoadFromMemory(black, 1, 1, 4);

	m_pWhiteTexture->LoadFromMemory(white, 1, 1, 4);

	m_pBlackTexture->AddRef();
	m_pWhiteTexture->AddRef();


	Vector2 size(512, 512);


	m_pFrontTexture = MTexture::CreateRenderTarget();
	m_pFrontTexture->SetSize(size);

	m_pBackTexture = MTexture::CreateRenderTarget();
	m_pBackTexture->SetSize(size);
	
	m_pFrontDepthForPassA = MTexture::CreateRenderTargetFloat32();
	m_pFrontDepthForPassA->SetSize(size);
	m_pFrontDepthForPassA->GenerateBuffer(pRenderSystem->GetDevice());

	m_pBackDepthForPassA = MTexture::CreateRenderTargetFloat32();
	m_pBackDepthForPassA->SetSize(size);
	m_pBackDepthForPassA->GenerateBuffer(pRenderSystem->GetDevice());

	m_pFrontDepthForPassB = MTexture::CreateRenderTargetFloat32();
	m_pFrontDepthForPassB->SetSize(size);
	m_pFrontDepthForPassB->GenerateBuffer(pRenderSystem->GetDevice());

	m_pBackDepthForPassB = MTexture::CreateRenderTargetFloat32();
	m_pBackDepthForPassB->SetSize(size);
	m_pBackDepthForPassB->GenerateBuffer(pRenderSystem->GetDevice());

	m_pDefaultOutputTexture = MTexture::CreateRenderTarget();
	m_pDefaultOutputTexture->SetSize(size);
	m_pDefaultOutputTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_pOutputTexture = m_pDefaultOutputTexture;
}

void MTransparentRenderWork::ReleaseTexture()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_pBlackTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());
	m_pWhiteTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());

	m_pBlackTexture->SubRef();
	m_pWhiteTexture->SubRef();

	m_pBlackTexture = nullptr;
	m_pWhiteTexture = nullptr;

	if (m_pFrontTexture)
	{
		m_pFrontTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pFrontTexture;
		m_pFrontTexture = nullptr;
	}

	if (m_pBackTexture)
	{
		m_pBackTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pBackTexture;
		m_pBackTexture = nullptr;
	}

	if (m_pFrontDepthForPassA)
	{
		m_pFrontDepthForPassA->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pFrontDepthForPassA;
		m_pFrontDepthForPassA = nullptr;
	}

	if (m_pBackDepthForPassA)
	{
		m_pBackDepthForPassA->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pBackDepthForPassA;
		m_pBackDepthForPassA = nullptr;
	}

	if (m_pFrontDepthForPassB)
	{
		m_pFrontDepthForPassB->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pFrontDepthForPassB;
		m_pFrontDepthForPassB = nullptr;
	}

	if (m_pBackDepthForPassB)
	{
		m_pBackDepthForPassB->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pBackDepthForPassB;
		m_pBackDepthForPassB = nullptr;
	}

	if (m_pDefaultOutputTexture)
	{
		m_pDefaultOutputTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pDefaultOutputTexture;
		m_pDefaultOutputTexture = nullptr;
	}
}

void MTransparentRenderWork::InitializePeelRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_peelRenderPass.AddBackTexture(m_pFrontTexture, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pBackTexture, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pFrontDepthForPassA, { true, MColor::White });
	
	m_peelRenderPass.AddBackTexture(m_pBackDepthForPassA, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pFrontDepthForPassB, { true, MColor::White });
	
	m_peelRenderPass.AddBackTexture(m_pBackDepthForPassB, { true, MColor::Black_T });


	static const uint32_t SUB_PASS_NUM = 6;

	m_peelRenderPass.m_vSubpass.push_back(MSubpass());
	MSubpass& subpass = m_peelRenderPass.m_vSubpass.back();

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
		m_peelRenderPass.m_vSubpass.push_back(MSubpass());
		MSubpass& subpass = m_peelRenderPass.m_vSubpass.back();

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


	m_peelRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::ReleasePeelRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_peelRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeFillRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();


	m_fillRenderPass.AddBackTexture(m_pOutputTexture, { false, true, MColor::Black_T });

	m_fillRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::ReleaseFillRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_fillRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeFrameShaderParams()
{
	m_aFrameParamSet[0].InitializeShaderParamSet(GetEngine());
	m_aFrameParamSet[0].m_pTransparentFrontTextureParam->pTexture = m_pFrontDepthForPassB;
	m_aFrameParamSet[0].m_pTransparentFrontTextureParam->SetDirty();
	m_aFrameParamSet[0].m_pTransparentBackTextureParam->pTexture = m_pBackDepthForPassB;
	m_aFrameParamSet[0].m_pTransparentBackTextureParam->SetDirty();

	m_aFrameParamSet[1].InitializeShaderParamSet(GetEngine());
	m_aFrameParamSet[1].m_pTransparentFrontTextureParam->pTexture = m_pFrontDepthForPassA;
	m_aFrameParamSet[1].m_pTransparentFrontTextureParam->SetDirty();
	m_aFrameParamSet[1].m_pTransparentBackTextureParam->pTexture = m_pBackDepthForPassA;
	m_aFrameParamSet[1].m_pTransparentBackTextureParam->SetDirty();
}

void MTransparentRenderWork::ReleaseFrameShaderParams()
{
	m_aFrameParamSet[0].ReleaseShaderParamSet(GetEngine());
	m_aFrameParamSet[1].ReleaseShaderParamSet(GetEngine());
}
