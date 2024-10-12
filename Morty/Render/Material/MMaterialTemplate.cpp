#include "Material/MMaterialTemplate.h"

#include "Engine/MEngine.h"
#include "MMaterial.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterialTemplate, MResource)

void MMaterialTemplate::SetCullMode(const MECullMode& eType)
{
    if (m_cullMode == eType) return;

    m_cullMode = eType;
}

void MMaterialTemplate::SetMaterialType(const MEMaterialType& eType)
{
    if (m_materialType == eType) return;

    m_materialType = eType;
}

void MMaterialTemplate::SetShaderMacro(const MShaderMacro& macro) { m_shaderProgram->SetShaderMacro(macro); }

void MMaterialTemplate::SetShadingRate(const Vector2i n2ShadingRate) { m_shadingRate = n2ShadingRate; }

void MMaterialTemplate::AddDefine(const MStringId& strKey, const MString& strValue)
{
    m_shaderProgram->GetShaderMacro().AddUnionMacro(strKey, strValue);
}

bool MMaterialTemplate::LoadShader(std::shared_ptr<MResource> pResource)
{
    return m_shaderProgram->LoadShader(pResource);
}

bool MMaterialTemplate::LoadShader(const MString& strResource)
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
    {
        return LoadShader(pResource);
    }

    return false;
}

void MMaterialTemplate::OnCreated()
{
    Super::OnCreated();

    m_shaderProgram = MShaderProgram::MakeShared(GetEngine(), MShaderProgram::EUsage::EGraphics);
}

void MMaterialTemplate::OnDelete()
{
    m_shaderProgram = nullptr;

    Super::OnDelete();
}

std::shared_ptr<MShaderPropertyBlock>
MMaterialTemplate::CreateFramePropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram)
{
    return pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_FRAME]->Clone();
}

std::shared_ptr<MShaderPropertyBlock>
MMaterialTemplate::CreateMeshPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram)
{
    return pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MESH]->Clone();
}

std::shared_ptr<MShaderPropertyBlock>
MMaterialTemplate::CreateMaterialPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram)
{
    return pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->Clone();
}
