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


const int SpecularMipmapCount = 7;
const float EnvironmentTextureSize = 128.0f;

MORTY_CLASS_IMPLEMENT(MEnvironmentMapRenderWork, MObject)

MEnvironmentMapRenderWork::MEnvironmentMapRenderWork()
	: MObject()
	, m_bUpdateNextFrame(true)
	, m_DiffuseMaterial(nullptr)
	, m_vSpecularMaterial()
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

	
	RenderDiffuse(pCommand, pSkyBoxComponent);
	RenderSpecular(pCommand, pSkyBoxComponent);
	
	m_bUpdateNextFrame = false;
}

MResource* MEnvironmentMapRenderWork::GetDiffuseOutputTexture()
{
	return m_DiffuseEnvironmentMap.GetResource();
}

void MEnvironmentMapRenderWork::RenderDiffuse(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MResource* pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();
	if (m_DiffuseMaterial)
	{
		m_DiffuseMaterial->SetTexutreParam("U_SkyBox", pSkyBoxTexture);
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
}

void MEnvironmentMapRenderWork::RenderSpecular(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MResource* pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();

	MTexture* pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>()->GetTextureTemplate();
	

	for (uint32_t nIdx = 0; nIdx < m_vSpecularRenderPass.size(); ++nIdx)
	{
		if (m_vSpecularMaterial[nIdx])
		{
			m_vSpecularMaterial[nIdx]->SetTexutreParam("U_SkyBox", pSkyBoxTexture);
		}

		pCommand->BeginRenderPass(&m_vSpecularRenderPass[nIdx]);

		Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
		Vector2 v2Size = pSpecularTexture->GetMipmapSize(nIdx);

		pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
		pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

		pCommand->SetUseMaterial(m_vSpecularMaterial[nIdx]);
		pCommand->DrawMesh(m_pCubeMesh->GetMesh());

		pCommand->EndRenderPass();
	}

	if (MTextureResource* pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>())
	{
		if (MTexture* pTexture = pSpecularTexture->GetTextureTemplate())
		{
			pCommand->SetRenderToTextureBarrier({ pTexture });
		}

		pSkyBoxComponent->LoadSpecularEnvResource(pSpecularTexture);
	}
}

void MEnvironmentMapRenderWork::InitializeResource()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pCubeMesh = pResourceSystem->CreateResource<MMeshResource>("Environment Draw Mesh");
	m_pCubeMesh->LoadAsSphere();
	m_pCubeMesh->AddRef();

	if (MTextureResource* pDiffuseCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
	{
		pDiffuseCubeMapResource->CreateCubeMapRenderTarget(EnvironmentTextureSize, EnvironmentTextureSize, 4, METextureLayout::ERGBA_FLOAT_16, false);

		m_DiffuseEnvironmentMap.SetResource(pDiffuseCubeMapResource);
	}

	if (MTextureResource* pSpecularCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
	{
		pSpecularCubeMapResource->CreateCubeMapRenderTarget(EnvironmentTextureSize, EnvironmentTextureSize, 4, METextureLayout::ERGBA_FLOAT_16, true);

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
	MResource* vs = pResourceSystem->LoadResource("Shader/ibl_map.mvs");
	MResource* diffuseps = pResourceSystem->LoadResource("Shader/diffuse_map.mps");
	m_DiffuseMaterial->LoadVertexShader(vs);
	m_DiffuseMaterial->LoadPixelShader(diffuseps);
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


	MResource* specularps = pResourceSystem->LoadResource("Shader/specular_map.mps");
	m_vSpecularMaterial.resize(SpecularMipmapCount);
	for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
	{
		m_vSpecularMaterial[nMipmap] = pResourceSystem->CreateResource<MMaterial>(MString("Specular CubeMap Material_") + MStringHelper::ToString(nMipmap));
		m_vSpecularMaterial[nMipmap]->LoadVertexShader(vs);
		m_vSpecularMaterial[nMipmap]->LoadPixelShader(specularps);
		m_vSpecularMaterial[nMipmap]->AddRef();
		m_vSpecularMaterial[nMipmap]->SetRasterizerType(MERasterizerType::ECullFront);

		if (MShaderParamSet* pParams = m_vSpecularMaterial[nMipmap]->GetMaterialParamSet())
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
			if (MStruct* matrix = pParams->m_vParams[1]->var.GetStruct())
			{
				if (float* roughness = matrix->FindMember<float>("U_roughness"))
				{
					*roughness = (float)nMipmap / (float)(SpecularMipmapCount);
				}

				pParams->m_vParams[1]->SetDirty();
			}
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

	for (MMaterial* pMaterial : m_vSpecularMaterial)
	{
		pMaterial->SubRef();
		pMaterial = nullptr;
	}
	m_vSpecularMaterial.clear();
}

void MEnvironmentMapRenderWork::InitializeRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	MTextureResource* pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>();
	MTextureResource* pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>();

	m_DiffuseRenderPass.AddBackTexture(pDiffuseTexture->GetTextureTemplate(), { true, MColor::Black_T });

	m_vSpecularRenderPass.resize(SpecularMipmapCount);
	for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
	{
		m_vSpecularRenderPass[nMipmap].SetViewNum(6);
		m_vSpecularRenderPass[nMipmap].AddBackTexture(pSpecularTexture->GetTextureTemplate(), { true, false, MColor::Black_T, nMipmap });
		m_vSpecularRenderPass[nMipmap].GenerateBuffer(pRenderSystem->GetDevice());
	}

	m_DiffuseRenderPass.SetViewNum(6);
	m_DiffuseRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

}

void MEnvironmentMapRenderWork::ReleaseRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_DiffuseRenderPass.DestroyBuffer(pRenderSystem->GetDevice());

	for (MRenderPass& renderpass : m_vSpecularRenderPass)
	{
		renderpass.DestroyBuffer(pRenderSystem->GetDevice());
	}

	m_vSpecularRenderPass.clear();
}
