#include "MEnvironmentMapRenderWork.h"

#include "Math/MMath.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Basic/MViewport.h"
#include "Render/MRenderCommand.h"

#include "Material/MMaterial.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMeshResource.h"
#include "Resource/MTextureResource.h"

#include "Component/MSkyBoxComponent.h"
#include "Resource/MMeshResourceUtil.h"


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

std::shared_ptr<MResource> MEnvironmentMapRenderWork::GetDiffuseOutputTexture()
{
	return m_DiffuseEnvironmentMap.GetResource();
}

void MEnvironmentMapRenderWork::RenderDiffuse(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	std::shared_ptr<MResource> pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();
	if (m_DiffuseMaterial)
	{
		m_DiffuseMaterial->SetTexture("u_texSkyBox", pSkyBoxTexture);
	}


	pCommand->BeginRenderPass(&m_DiffuseRenderPass);

	Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
	Vector2 v2Size = Vector2(EnvironmentTextureSize, EnvironmentTextureSize);
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

	pCommand->SetUseMaterial(m_DiffuseMaterial);
	pCommand->DrawMesh(m_pCubeMesh->GetMesh());

	pCommand->EndRenderPass();


	if (std::shared_ptr<MTextureResource> pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>())
	{
		if (std::shared_ptr<MTexture> pTexture = pDiffuseTexture->GetTextureTemplate())
		{
			pCommand->AddRenderToTextureBarrier({ pTexture.get() });
		}

		pSkyBoxComponent->LoadDiffuseEnvResource(pDiffuseTexture);
	}
}

void MEnvironmentMapRenderWork::RenderSpecular(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	std::shared_ptr<MResource> pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();

	std::shared_ptr<MTexture> pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>()->GetTextureTemplate();
	

	for (uint32_t nIdx = 0; nIdx < m_vSpecularRenderPass.size(); ++nIdx)
	{
		if (m_vSpecularMaterial[nIdx])
		{
			m_vSpecularMaterial[nIdx]->SetTexture("u_texSkyBox", pSkyBoxTexture);
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

	if (std::shared_ptr<MTextureResource> pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>())
	{
		if (std::shared_ptr<MTexture> pTexture = pSpecularTexture->GetTextureTemplate())
		{
			pCommand->AddRenderToTextureBarrier({ pTexture.get()});
		}

		pSkyBoxComponent->LoadSpecularEnvResource(pSpecularTexture);
	}
}

void MEnvironmentMapRenderWork::InitializeResource()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pCubeMesh = pResourceSystem->CreateResource<MMeshResource>("Environment Draw Mesh");

	m_pCubeMesh->Load(MMeshResourceUtil::CreateSphere());

	if (std::shared_ptr<MTextureResource> pDiffuseCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
	{
		pDiffuseCubeMapResource->CreateCubeMapRenderTarget(EnvironmentTextureSize, EnvironmentTextureSize, 4, METextureLayout::ERGBA_FLOAT_16, false);

		m_DiffuseEnvironmentMap.SetResource(pDiffuseCubeMapResource);
	}

	if (std::shared_ptr<MTextureResource> pSpecularCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
	{
		pSpecularCubeMapResource->CreateCubeMapRenderTarget(EnvironmentTextureSize, EnvironmentTextureSize, 4, METextureLayout::ERGBA_FLOAT_16, true);

		m_SpecularEnvironmentMap.SetResource(pSpecularCubeMapResource);
	}
}

void MEnvironmentMapRenderWork::ReleaseResource()
{
	m_pCubeMesh = nullptr;

	m_DiffuseEnvironmentMap.SetResource(nullptr);
	m_SpecularEnvironmentMap.SetResource(nullptr);
}

void MEnvironmentMapRenderWork::InitializeMaterial()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	Matrix4 m4Projection = MRenderSystem::MatrixPerspectiveFovLH(45.0f, 1.0f, 0.1f, 100.0f);

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
	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/ibl_map.mvs");
	std::shared_ptr<MResource> diffuseps = pResourceSystem->LoadResource("Shader/diffuse_map.mps");
	m_DiffuseMaterial->LoadVertexShader(vs);
	m_DiffuseMaterial->LoadPixelShader(diffuseps);

	m_DiffuseMaterial->SetCullMode(MECullMode::ECullFront);

	if (const std::shared_ptr<MShaderPropertyBlock>& pParams = m_DiffuseMaterial->GetMaterialPropertyBlock())
	{
		MVariantStruct& matrix = pParams->m_vParams[0]->var.GetValue<MVariantStruct>();
		{
			MVariantArray& mvp = matrix.GetVariant<MVariantArray>("u_ModelViewProj");
			for (uint32_t i = 0; i < 6; ++i)
			{
				mvp.SetVariant(i, vCmaeraView[i]);
			}

			pParams->m_vParams[0]->SetDirty();
		}
	}


	std::shared_ptr<MResource> specularps = pResourceSystem->LoadResource("Shader/specular_map.mps");
	m_vSpecularMaterial.resize(SpecularMipmapCount);
	for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
	{
		m_vSpecularMaterial[nMipmap] = pResourceSystem->CreateResource<MMaterial>(MString("Specular CubeMap Material_") + MStringHelper::ToString(nMipmap));
		m_vSpecularMaterial[nMipmap]->LoadVertexShader(vs);
		m_vSpecularMaterial[nMipmap]->LoadPixelShader(specularps);
		m_vSpecularMaterial[nMipmap]->SetCullMode(MECullMode::ECullFront);

		if (const std::shared_ptr<MShaderPropertyBlock>& pParams = m_vSpecularMaterial[nMipmap]->GetMaterialPropertyBlock())
		{
			{
				MVariantStruct& matrix = pParams->m_vParams[0]->var.GetValue<MVariantStruct>();
				MVariantArray& mvp = matrix.GetVariant<MVariantArray>("u_ModelViewProj");
				
				for (uint32_t i = 0; i < 6; ++i)
				{
					mvp.SetVariant(i, vCmaeraView[i]);
				}

				pParams->m_vParams[0]->SetDirty();
			}

			{
				MVariantStruct& matrix = pParams->m_vParams[1]->var.GetValue<MVariantStruct>();
				matrix.SetVariant<float>("u_roughness", (float)nMipmap / (float)(SpecularMipmapCount));
				pParams->m_vParams[1]->SetDirty();
			}
		}
	}

}

void MEnvironmentMapRenderWork::ReleaseMaterial()
{
	if (m_DiffuseMaterial)
	{
		m_DiffuseMaterial = nullptr;
	}

	for (std::shared_ptr<MMaterial> pMaterial : m_vSpecularMaterial)
	{
		pMaterial = nullptr;
	}
	m_vSpecularMaterial.clear();
}

void MEnvironmentMapRenderWork::InitializeRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::shared_ptr<MTextureResource> pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>();
	std::shared_ptr<MTextureResource> pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>();

	m_DiffuseRenderPass.AddBackTexture(pDiffuseTexture->GetTextureTemplate(), { true, MColor::Black_T });

	m_vSpecularRenderPass.resize(SpecularMipmapCount);
	for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
	{
		m_vSpecularRenderPass[nMipmap].SetViewportNum(6);
		m_vSpecularRenderPass[nMipmap].AddBackTexture(pSpecularTexture->GetTextureTemplate(), {true, false, MColor::Black_T, nMipmap});
		m_vSpecularRenderPass[nMipmap].GenerateBuffer(pRenderSystem->GetDevice());
	}

	m_DiffuseRenderPass.SetViewportNum(6);
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
