#include "MEnvironmentMapRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Math/MMath.h"
#include "RHI/IRenderCommand.h"
#include "Scene/MEntity.h"

#include "Material/MMaterial.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMeshResource.h"
#include "Resource/MTextureResource.h"

#include "Component/MSkyBoxComponent.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "Resource/MMeshResourceUtil.h"

using namespace morty;

const int   SpecularMipmapCount    = 7;
const float EnvironmentTextureSize = 128.0f;

MORTY_CLASS_IMPLEMENT(MEnvironmentMapRenderNode, MObject)

MEnvironmentMapRenderNode::MEnvironmentMapRenderNode()
    : MObject()
    , m_updateNextFrame(true)
    , m_DiffuseMaterial(nullptr)
    , m_specularMaterial()
    , m_cubeMesh(nullptr)
{}

void MEnvironmentMapRenderNode::OnCreated()
{
    Super::OnCreated();
    InitializeResource();
    InitializeMaterial();
    InitializeRenderPass();
}

void MEnvironmentMapRenderNode::OnDelete()
{
    ReleaseRenderPass();
    ReleaseMaterial();
    ReleaseResource();
    Super::OnDelete();
}

void MEnvironmentMapRenderNode::MarkUpdateEnvironment() { m_updateNextFrame = true; }

void MEnvironmentMapRenderNode::RenderEnvironment(IRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
    if (!m_updateNextFrame) return;

    if (!pCommand) return;

    if (!pSkyBoxComponent) return;


    RenderDiffuse(pCommand, pSkyBoxComponent);
    RenderSpecular(pCommand, pSkyBoxComponent);

    m_updateNextFrame = false;
}

std::shared_ptr<MResource> MEnvironmentMapRenderNode::GetDiffuseOutputTexture() const
{
    return m_DiffuseEnvironmentMap.GetResource();
}

void MEnvironmentMapRenderNode::RenderDiffuse(IRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{

    std::shared_ptr<MResource> pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();
    if (m_DiffuseMaterial)
    {
        m_DiffuseMaterial->SetTexture(MShaderPropertyName::ENVIRONMENT_TEXTURE_SKYBOX, pSkyBoxTexture);
    }


    auto command = pCommand->BeginRenderPass(&m_DiffuseRenderPass);

    command.SetViewportAndScissor(
            {.x = 0.0f, .y = 0.0f, .width = EnvironmentTextureSize, .height = EnvironmentTextureSize}
    );

    command.SetMaterial(m_DiffuseMaterial.get());
    command.DrawMesh(m_cubeMesh->GetMesh());

    pCommand->EndRenderPass(command);

    if (std::shared_ptr<MTextureResource> pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>())
    {
        pSkyBoxComponent->LoadDiffuseEnvResource(pDiffuseTexture);
    }
}

void MEnvironmentMapRenderNode::RenderSpecular(IRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
    std::shared_ptr<MResource> pSkyBoxTexture         = pSkyBoxComponent->GetSkyBoxResource();
    auto                       pSkyBoxTextureResource = MTypeClass::DynamicCast<MTextureResource>(pSkyBoxTexture);

    MTexturePtr pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>()->GetTextureTemplate();

    for (uint32_t nIdx = 0; nIdx < m_specularRenderPass.size(); ++nIdx)
    {
        if (m_specularBlock[nIdx])
        {
            m_specularBlock[nIdx]->SetTexture(
                    MShaderPropertyName::ENVIRONMENT_TEXTURE_SKYBOX,
                    pSkyBoxTextureResource->GetTextureTemplate()
            );
        }

        MRenderPassCmd command = pCommand->BeginRenderPass(&m_specularRenderPass[nIdx]);
        Vector2        v2Size  = pSpecularTexture->GetMipmapSize(nIdx);

        command.SetViewportAndScissor({.x = 0.0f, .y = 0.0f, .width = v2Size.x, .height = v2Size.y});
        command.SetGraphPipeline(m_specularMaterial.get());
        command.SetShaderPropertyBlock(m_specularBlock[nIdx].get());

        command.DrawMesh(m_cubeMesh->GetMesh());

        pCommand->EndRenderPass(command);
    }

    if (std::shared_ptr<MTextureResource> texture = m_SpecularEnvironmentMap.GetResource<MTextureResource>())
    {
        pSkyBoxComponent->LoadSpecularEnvResource(texture);
    }
}

void MEnvironmentMapRenderNode::InitializeResource()
{
    auto* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    m_cubeMesh = pResourceSystem->CreateResource<MMeshResource>("Environment Draw Mesh");

    m_cubeMesh->Load(MMeshResourceUtil::CreateSphere());

    if (std::shared_ptr<MTextureResource> pDiffuseCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
    {
        pDiffuseCubeMapResource->CreateCubeMapRenderTarget(
                EnvironmentTextureSize,
                EnvironmentTextureSize,
                4,
                METextureFormat::Float_RGBA16,
                false
        );

        m_DiffuseEnvironmentMap.SetResource(pDiffuseCubeMapResource);
    }

    if (std::shared_ptr<MTextureResource> pSpecularCubeMapResource =
                pResourceSystem->CreateResource<MTextureResource>())
    {
        pSpecularCubeMapResource->CreateCubeMapRenderTarget(
                EnvironmentTextureSize,
                EnvironmentTextureSize,
                4,
                METextureFormat::Float_RGBA16,
                true
        );

        m_SpecularEnvironmentMap.SetResource(pSpecularCubeMapResource);
    }
}

void MEnvironmentMapRenderNode::ReleaseResource()
{
    m_cubeMesh = nullptr;

    m_DiffuseEnvironmentMap.SetResource(nullptr);
    m_SpecularEnvironmentMap.SetResource(nullptr);
}

void MEnvironmentMapRenderNode::InitializeMaterial()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    Matrix4          m4Projection = MRenderSystem::MatrixPerspectiveFovLH(90.0f, 1.0f, 0.1f, 100.0f);

    Matrix4          vCmaeraView[6] = {
            MMath::LookAt(Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),
            MMath::LookAt(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),
            MMath::LookAt(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
            MMath::LookAt(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),
            MMath::LookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),
            MMath::LookAt(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),
    };

    for (uint32_t i = 0; i < 6; ++i) { vCmaeraView[i] = m4Projection * vCmaeraView[i]; }

    m_DiffuseMaterial             = pResourceSystem->CreateResource<MMaterialTemplate>("Diffuse CubeMap Material");
    std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/IBL/ibl_map.mvs");
    std::shared_ptr<MResource> diffuseps = pResourceSystem->LoadResource("Shader/IBL/diffuse_map.mps");
    m_DiffuseMaterial->LoadShader(vs);
    m_DiffuseMaterial->LoadShader(diffuseps);
    m_DiffuseMaterial->SetCullMode(MECullMode::ECullFront);

    if (const std::shared_ptr<MShaderPropertyBlock>& pParams = m_DiffuseMaterial->GetMaterialPropertyBlock())
    {
        MVariantStruct& matrix = pParams->m_params[0]->var.GetValue<MVariantStruct>();
        {
            MVariantArray& mvp = matrix.GetVariant<MVariantArray>(MShaderPropertyName::ENVIRONMENT_IBL_MVP_MATRIX);
            for (uint32_t i = 0; i < 6; ++i) { mvp.SetVariant(i, vCmaeraView[i]); }

            pParams->m_params[0]->SetDirty();
        }
    }


    std::shared_ptr<MResource> specularps = pResourceSystem->LoadResource("Shader/IBL/specular_map.mps");
    m_specularMaterial = pResourceSystem->CreateResource<MMaterialTemplate>(MString("Specular CubeMap Material"));
    m_specularMaterial->LoadShader(vs);
    m_specularMaterial->LoadShader(specularps);
    m_specularMaterial->SetCullMode(MECullMode::ECullFront);

    m_specularBlock.resize(SpecularMipmapCount);
    for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
    {
        m_specularBlock[nMipmap] =
                MMaterialTemplate::CreateMaterialPropertyBlock(m_specularMaterial->GetShaderProgram());

        if (const std::shared_ptr<MShaderPropertyBlock>& pParams = m_specularBlock[nMipmap])
        {
            {
                MVariantStruct& matrix = pParams->m_params[0]->var.GetValue<MVariantStruct>();
                MVariantArray&  mvp = matrix.GetVariant<MVariantArray>(MShaderPropertyName::ENVIRONMENT_IBL_MVP_MATRIX);

                for (uint32_t i = 0; i < 6; ++i) { mvp.SetVariant(i, vCmaeraView[i]); }

                pParams->m_params[0]->SetDirty();
            }

            {
                MVariantStruct& matrix = pParams->m_params[1]->var.GetValue<MVariantStruct>();
                matrix.SetVariant<float>(
                        MShaderPropertyName::ENVIRONMENT_IBL_ROUGHNESS,
                        (float) nMipmap / (float) (SpecularMipmapCount)
                );
                pParams->m_params[1]->SetDirty();
            }
        }
    }
}

void MEnvironmentMapRenderNode::ReleaseMaterial()
{
    if (m_DiffuseMaterial) { m_DiffuseMaterial = nullptr; }

    for (auto pMaterial: m_specularBlock) { pMaterial = nullptr; }
    m_specularBlock.clear();
}

void MEnvironmentMapRenderNode::InitializeRenderPass()
{
#if MORTY_DEBUG
    m_DiffuseRenderPass.m_strDebugName = "EnvironmentMap Diffuse";
#endif

    MRenderSystem*                    pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    std::shared_ptr<MTextureResource> pDiffuseTexture  = m_DiffuseEnvironmentMap.GetResource<MTextureResource>();
    std::shared_ptr<MTextureResource> pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>();

    m_DiffuseRenderPass.AddBackTexture(pDiffuseTexture->GetTextureTemplate(), {true, MColor::Black_T});

    m_specularRenderPass.resize(SpecularMipmapCount);
    for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
    {
        m_specularRenderPass[nMipmap].SetViewportNum(6);
        m_specularRenderPass[nMipmap].AddBackTexture(
                pSpecularTexture->GetTextureTemplate(),
                {true, MColor::Black_T, nMipmap}
        );
        m_specularRenderPass[nMipmap].GenerateBuffer(pRenderSystem->GetDevice());
    }

    m_DiffuseRenderPass.SetViewportNum(6);
    m_DiffuseRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MEnvironmentMapRenderNode::ReleaseRenderPass()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_DiffuseRenderPass.DestroyBuffer(pRenderSystem->GetDevice());

    for (MRenderPass& renderpass: m_specularRenderPass) { renderpass.DestroyBuffer(pRenderSystem->GetDevice()); }

    m_specularRenderPass.clear();
}
