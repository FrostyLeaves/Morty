#include "Resource/MShaderResource.h"
#include "Engine/MEngine.h"
#include "Math/MMath.h"
#include "RHI/Abstract/MIDevice.h"
#include "Shader/MShader.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShaderResource, MResource)

MShader* MShaderResource::GetShaderByIndex(const int& nIndex)
{
    int nSize = static_cast<int>(m_shaders.size()) - 1;
    return m_shaders[MMath::Clamp(nIndex, 0, nSize)];
}

int MShaderResource::FindShaderByMacroParam(const MShaderMacro& macro)
{
    auto pShaderData = static_cast<MShaderResourceData*>(m_resourceData.get());

    int  nSize = static_cast<int>(m_shaders.size());
    for (int i = 0; i < nSize; ++i)
    {
        if (m_shaders[i]->m_shaderMacro.Compare(macro)) return i;
    }

    auto* pNewShader          = new MShader();
    pNewShader->m_shaderType  = pShaderData->shaderType;
    pNewShader->m_shaderPath  = pShaderData->shaderPath;
    pNewShader->m_entryName   = pShaderData->entryName;
    pNewShader->m_shaderMacro = macro;
    m_shaders.push_back(pNewShader);

    return static_cast<int>(m_shaders.size()) - 1;
}

MEShaderType MShaderResource::GetShaderType() const
{
    if (auto ptr = static_cast<MShaderResourceData*>(m_resourceData.get())) { return ptr->shaderType; }

    return MEShaderType::ENone;
}

bool MShaderResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
    auto  pShaderData = static_cast<MShaderResourceData*>(pResourceData.get());

    auto* pRenderSystem = m_engine->FindSystem<MRenderSystem>();

    for (MShader* pShader: m_shaders)
    {
        pShader->CleanShader(pRenderSystem->GetDevice());

        pShader->m_shaderPath = pShaderData->shaderPath;
        pShader->m_entryName  = pShaderData->entryName;
        pShader->m_shaderType = pShaderData->shaderType;
    }

    m_resourceData = std::move(pResourceData);

    return true;
}

bool MShaderResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    MORTY_UNUSED(pResourceData);
    return false;
}

void MShaderResource::OnDelete()
{
    auto* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    MORTY_ASSERT(pRenderSystem);

    for (MShader* pShader: m_shaders)
    {
        pShader->CleanShader(pRenderSystem->GetDevice());
        delete pShader;
        pShader = nullptr;
    }

    m_shaders.clear();


    MResource::OnDelete();
}

const MType*                   MShaderResourceLoader::ResourceType() const { return MShaderResource::GetClassType(); }

std::unique_ptr<MResourceData> MShaderResourceLoader::LoadResource(const MString& svFullPath)
{
    std::unique_ptr<MShaderResourceData>   pResourceData = std::make_unique<MShaderResourceData>();

    const MString                          strPathSuffix = MResource::GetSuffix(svFullPath);

    static std::map<MString, MEShaderType> ShaderSuffixTable = {
            {MRenderGlobal::SUFFIX_VERTEX_SHADER, MEShaderType::EVertex},
            {MRenderGlobal::SUFFIX_PIXEL_SHADER, MEShaderType::EPixel},
            {MRenderGlobal::SUFFIX_COMPUTE_SHADER, MEShaderType::ECompute},
            {MRenderGlobal::SUFFIX_GEOMETRY_SHADER, MEShaderType::EGeometry},
    };

    static std::map<MString, MString> ShaderEntryTable = {
            {MRenderGlobal::SUFFIX_VERTEX_SHADER, "VS_MAIN"},
            {MRenderGlobal::SUFFIX_PIXEL_SHADER, "PS_MAIN"},
            {MRenderGlobal::SUFFIX_COMPUTE_SHADER, "CS_MAIN"},
            {MRenderGlobal::SUFFIX_GEOMETRY_SHADER, "GS_MAIN"},
    };


    MORTY_ASSERT(ShaderSuffixTable.find(strPathSuffix) != ShaderSuffixTable.end());

    pResourceData->shaderType = ShaderSuffixTable[strPathSuffix];
    pResourceData->entryName  = ShaderEntryTable[strPathSuffix];

    //TODO load as buffer.
    pResourceData->shaderPath = svFullPath;
    return pResourceData;
}

void MShaderResourceData::LoadBuffer(const std::vector<MByte>& buffer)
{
    MORTY_UNUSED(buffer);
    MORTY_ASSERT(false);
}

std::vector<MByte> MShaderResourceData::SaveBuffer() const { return std::vector<MByte>(); }
