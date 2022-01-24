#include "MEnvironmentMapRenderWork.h"

#include "MMath.h"
#include "MEngine.h"
#include "MEntity.h"
#include "MViewport.h"
#include "MRenderCommand.h"

#include "MMaterial.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MMeshResource.h"
#include "MTextureResource.h"

#include "MSkyBoxComponent.h"


const float EnvironmentTextureSize = 128.0f;

MORTY_CLASS_IMPLEMENT(MEnvironmentMapRenderWork, MObject)

MEnvironmentMapRenderWork::MEnvironmentMapRenderWork()
	: MObject()
	, m_bUpdateNextFrame(true)
	, m_DiffuseMaterial(nullptr)
	, m_SpecularMaterial(nullptr)
	, m_pCubeMesh(nullptr)
{

}

MEnvironmentMapRenderWork::~MEnvironmentMapRenderWork()
{

}

void MEnvironmentMapRenderWork::OnCreated()
{
	Super::OnCreated();
	InitializeResource();
	InitializeMaterial();
	InitializeRenderPass();
}

void MEnvironmentMapRenderWork::OnDelete()
{
	ReleaseRenderPass();
	ReleaseMaterial();
	ReleaseResource();
	Super::OnDelete();
}

void MEnvironmentMapRenderWork::MarkUpdateEnvironment()
{
	m_bUpdateNextFrame = true;
}

void MEnvironmentMapRenderWork::RenderEnvironment(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
	if (!m_bUpdateNextFrame)
		return;

	if (!pCommand)
		return;

	if (!pSkyBoxComponent)
		return;

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MResource* pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();
	if (m_DiffuseMaterial)
	{
		m_DiffuseMaterial->SetTexutreParam("U_SkyBox", pSkyBoxTexture);
	}

	if (m_SpecularMaterial)
	{
		m_SpecularMaterial->SetTexutreParam("U_SkyBox", pSkyBoxTexture);
	}

	pCommand->BeginRenderPass(&m_DiffuseRenderPass);
	
	Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
	Vector2 v2Size = Vector2(EnvironmentTextureSize, EnvironmentTextureSize);
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

	pCommand->SetUseMaterial(m_DiffuseMaterial);
	pCommand->DrawMesh(m_pCubeMesh->GetMesh());

	pCommand->EndRenderPass();



	if (MTextureResource* pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>())
	{
		if (MTexture* pTexture = pDiffuseTexture->GetTextureTemplate())
		{
			pCommand->SetRenderToTextureBarrier({ pTexture });
		}

		pSkyBoxComponent->LoadDiffuseEnvResource(pDiffuseTexture);
	}
	
	m_bUpdateNextFrame = false;
}

MResource* MEnvironmentMapRenderWork::GetDiffuseOutputTexture()
{
	return m_DiffuseEnvironmentMap.GetResource();
}

void MEnvironmentMapRenderWork::InitializeResource()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pCubeMesh = pResourceSystem->CreateResource<MMeshResource>("Environment Draw Mesh");
	m_pCubeMesh->LoadAsSphere();
	m_pCubeMesh->AddRef();

	if (MTextureResource* pDiffuseCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
	{
		pDiffuseCubeMapResource->CreateCubeMapRenderTarget(EnvironmentTextureSize, EnvironmentTextureSize, 4, METextureLayout::ERGBA_FLOAT_32);

		m_DiffuseEnvironmentMap.SetResource(pDiffuseCubeMapResource);
	}

	if (MTextureResource* pSpecularCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
	{
		pSpecularCubeMapResource->CreateCubeMapRenderTarget(EnvironmentTextureSize, EnvironmentTextureSize, 4, METextureLayout::ERGBA_FLOAT_32);

		m_SpecularEnvironmentMap.SetResource(pSpecularCubeMapResource);
	}
}

void MEnvironmentMapRenderWork::ReleaseResource()
{
	m_pCubeMesh->SubRef();
	m_pCubeMesh = nullptr;

	m_DiffuseEnvironmentMap.SetResource(nullptr);
	m_SpecularEnvironmentMap.SetResource(nullptr);
}

void MEnvironmentMapRenderWork::InitializeMaterial()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	Matrix4 m4Projection = MViewport::MatrixPerspectiveFovLH(45.0f, 1.0f, 0.1f, 100.0f);

	Matrix4 vCmaeraView[6] = {
		MMath::LookAt(Vector3(-1.0f,  0.0f,  0.0f), Vector3(0.0f, 1.0f,  0.0f)),
		MMath::LookAt(Vector3(1.0f,  0.0f,  0.0f), Vector3(0.0f, 1.0f,  0.0f)),
		MMath::LookAt(Vector3(0.0f, -1.0f,  0.0f), Vector3(0.0f,  0.0f, 1.0f)),
		MMath::LookAt(Vector3(0.0f,  1.0f,  0.0f), Vector3(0.0f,  0.0f, -1.0f)),
		MMath::LookAt(Vector3(0.0f,  0.0f,  1.0f), Vector3(0.0f, 1.0f,  0.0f)),
		MMath::LookAt(Vector3(0.0f,  0.0f, -1.0f), Vector3(0.0f, 1.0f,  0.0f)),
	};

	for (uint32_t i = 0; i < 6; ++i)
	{
		vCmaeraView[i] = m4Projection * vCmaeraView[i];
	}

	m_DiffuseMaterial = pResourceSystem->CreateResource<MMaterial>("Diffuse CubeMap Material");
	MResource* vs = pResourceSystem->LoadResource("Shader/diffuse_map.mvs");
	MResource* ps = pResourceSystem->LoadResource("Shader/diffuse_map.mps");
	m_DiffuseMaterial->LoadVertexShader(vs);
	m_DiffuseMaterial->LoadPixelShader(ps);
	m_DiffuseMaterial->AddRef();

	m_DiffuseMaterial->SetRasterizerType(MERasterizerType::ECullFront);

	if (MShaderParamSet* pParams = m_DiffuseMaterial->GetMaterialParamSet())
	{
		if (MStruct* matrix = pParams->m_vParams[0]->var.GetStruct())
		{
			if (MVariantArray* mvp = matrix->FindMember<MVariantArray>("U_ModelViewProj"))
			{
				for (uint32_t i = 0; i < 6; ++i)
				{
					mvp->GetMember(i)->var = vCmaeraView[i];
				}
			}

			pParams->m_vParams[0]->SetDirty();
		}
	}
}

void MEnvironmentMapRenderWork::ReleaseMaterial()
{
	if (m_DiffuseMaterial)
	{
		m_DiffuseMaterial->SubRef();
		m_DiffuseMaterial = nullptr;
	}

	if (m_SpecularMaterial)
	{
		m_SpecularMaterial->SubRef();
		m_SpecularMaterial = nullptr;
	}
}

void MEnvironmentMapRenderWork::InitializeRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	MTextureResource* pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>();
	MTextureResource* pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>();

	m_DiffuseRenderPass.m_vBackTextures.push_back(pDiffuseTexture->GetTextureTemplate());
	m_DiffuseRenderPass.m_vBackDesc.push_back({ true, MColor::Black_T });

	m_SpecularRenderPass.m_vBackTextures.push_back(pSpecularTexture->GetTextureTemplate());
	m_SpecularRenderPass.m_vBackDesc.push_back({ true, MColor::Black_T });

	m_DiffuseRenderPass.SetViewNum(6);
	m_DiffuseRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

	m_SpecularRenderPass.SetViewNum(6);
	m_SpecularRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MEnvironmentMapRenderWork::ReleaseRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_DiffuseRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
	m_SpecularRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}
