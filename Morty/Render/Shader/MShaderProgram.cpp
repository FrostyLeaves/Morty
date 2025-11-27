#include "MShaderProgram.h"
#include "Engine/MEngine.h"
#include "MShaderBuffer.h"
#include "RHI/Abstract/MIDevice.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Shader/MShader.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"
#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShaderProgram, IShaderProgram)

MShaderProgram::MShaderProgram(
        MEngine*                          engine,
        EUsage                            usage,
        const std::shared_ptr<MResource>& shader,
        const MShaderMacro&               macro,
        const MEntryNames&                entryNames,
        uint8_t                           shaderMask
)
    : m_shaderResource(shader)
    , m_shaderMacro(macro)
    , m_engine(engine)
    , m_usage(usage)
    , m_entryNames(entryNames)
    , m_shaderMask(shaderMask)
{
    InitializeShaderParameterSet();
    MORTY_ASSERT(LoadShader(shader));

    CompileShaderIfNeed();
}

MShaderProgram::~MShaderProgram() { UnloadShader(); }

bool MShaderProgram::LoadShader(const std::shared_ptr<MResource>& pResource)
{
    if (std::shared_ptr<MShaderResource> pShaderResource = MTypeClass::DynamicCast<MShaderResource>(pResource))
    {
        auto LoadFunc = [this]() {
            for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
            {
                m_compiledShaders[nIdx].state = ShaderState::Unknow;
            }
            return true;
        };

        m_shaderResource.SetResource(pResource);
        m_shaderResource.SetResChangedCallback(LoadFunc);

        LoadFunc();
        return true;
    }

    return false;
}

void MShaderProgram::InitializeShaderParameterSet()
{
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] =
            std::make_shared<MShaderParameterSet>(this, MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] =
            std::make_shared<MShaderParameterSet>(this, MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] =
            std::make_shared<MShaderParameterSet>(this, MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_OTHER] =
            std::make_shared<MShaderParameterSet>(this, MRenderGlobal::SHADER_PARAM_SET_OTHER);
}

void MShaderProgram::UnloadShader()
{
    auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    for (size_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
    {
        m_shaderSets[i]->DestroyBuffer(pRenderSystem->GetDevice());
        m_shaderSets[i] = nullptr;
    }

    for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
    {
        m_compiledShaders[nIdx].pShader    = nullptr;
        m_compiledShaders[nIdx].nShaderIdx = 0;
    }

    m_shaderResource.SetResource(nullptr);
}

void MShaderProgram::CopyShaderParams(
        MEngine*                                          pEngine,
        const std::shared_ptr<MShaderParameterSet>&       target,
        const std::shared_ptr<const MShaderParameterSet>& source
)
{
    auto* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

    target->DestroyBuffer(pRenderSystem->GetDevice());

    target->m_uniforms.resize(source->m_uniforms.size());
    for (uint32_t i = 0; i < source->m_uniforms.size(); ++i)
    {
        target->m_uniforms[i] = std::make_unique<MShaderUniformParam>(*source->m_uniforms[i]);
    }

    target->m_textures.resize(source->m_textures.size());
    for (uint32_t i = 0; i < source->m_textures.size(); ++i)
    {
        auto pSource = dynamic_cast<MTextureResourceParam*>(source->m_textures[i].get());
        auto pParam  = std::make_unique<MTextureResourceParam>(*pSource);

        pParam->SetTexture(pSource->GetTextureResource());

        target->m_textures[i] = std::move(pParam);
    }

    target->m_samplers.resize(source->m_samplers.size());
    for (uint32_t i = 0; i < source->m_samplers.size(); ++i)
    {
        target->m_samplers[i] = std::make_unique<MShaderSamplerParam>(*source->m_samplers[i]);
    }

    target->m_storages.resize(source->m_storages.size());
    for (uint32_t i = 0; i < source->m_storages.size(); ++i)
    {
        target->m_storages[i] = std::make_unique<MShaderStorageParam>(*source->m_storages[i]);
    }
}

std::shared_ptr<MShaderParameterSet> MShaderProgram::AllocShaderParameterSet(size_t nSetIdx)
{
    std::shared_ptr<MShaderParameterSet> pShaderParameterSet = m_shaderSets[nSetIdx]->Clone();
    m_shaderParameterSetInstance.insert(pShaderParameterSet);

    return pShaderParameterSet;
}

void MShaderProgram::ReleaseShaderParameterSet(const std::shared_ptr<MShaderParameterSet>& pShaderParameterSet)
{
    m_shaderParameterSetInstance.erase(pShaderParameterSet);
}

void MShaderProgram::CompileShaderIfNeed()
{
    auto* pShaderResource = m_shaderResource.GetResource()->template DynamicCast<MShaderResource>();
    if (nullptr == pShaderResource) { return; }

    bool compileAction = false;

    for (size_t idx = 0; idx < m_compiledShaders.size(); ++idx)
    {
        const auto shaderType = static_cast<MEShaderType>(idx);
        auto&      desc       = m_compiledShaders[idx];

        if ((m_shaderMask & 1 << idx) == 0) { continue; }
        if (desc.state != ShaderState::Unknow) { continue; }

        desc.nShaderIdx     = pShaderResource->FindShaderByMacroParam(m_entryNames[idx], shaderType, m_shaderMacro);
        desc.pShader        = pShaderResource->GetShaderByIndex(desc.nShaderIdx);
        auto* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
        if (desc.pShader && !desc.pShader->IsCompiled())
        {
            if (!desc.pShader->CompileShader(pRenderSystem->GetDevice()))
            {
                desc.pShader = nullptr;
                desc.state   = ShaderState::Failed;
            }
            else
            {
                desc.state    = ShaderState::Compiled;
                compileAction = true;
            }
        }

        if (desc.pShader)
        {
            UnbindShaderBuffer(shaderType, pRenderSystem->GetDevice());
            BindShaderBuffer(desc.pShader->GetBuffer(), shaderType);
        }
    }

    if (compileAction)
    {
        m_propertyBlock.Clear();
        for (const auto& desc: m_compiledShaders)
        {
            if (desc.state == ShaderState::Compiled) { m_propertyBlock.Merge(desc.pShader->GetPropertyBlock()); }
        }
    }
}

MStringId MShaderProgram::GetEntryName(MEShaderType shaderType) const
{
    return m_entryNames[static_cast<size_t>(shaderType)];
}

MShader* MShaderProgram::GetShader(MEShaderType eType)
{
    CompileShaderIfNeed();
    const auto idx  = static_cast<size_t>(eType);
    auto&      desc = m_compiledShaders[idx];
    return desc.pShader;
}

std::array<std::shared_ptr<MShaderParameterSet>, MRenderGlobal::SHADER_PARAM_SET_NUM>&
MShaderProgram::GetShaderParameterSets()
{
    CompileShaderIfNeed();
    return m_shaderSets;
}

MHashCode MShaderProgram::GetHashCode() const
{
    //TODO
    return 0;
}

bool MShaderProgram::IsValid() const
{
    for (auto compiledShader: m_compiledShaders)
    {
        if (compiledShader.state == ShaderState::Failed) { return false; }
    }

    return true;
}


MTextureResourceParam::MTextureResourceParam()
    : MShaderTextureParam()
    , m_TextureRef()
{}

MTextureResourceParam::MTextureResourceParam(const MShaderTextureParam& param)
    : MShaderTextureParam(param)
    , m_TextureRef()
{}

void MTextureResourceParam::SetTexture(MTexturePtr pTexture)
{
    m_TextureRef.SetResource(nullptr);

    MShaderTextureParam::SetTexture(pTexture);
}

void MTextureResourceParam::SetTexture(const std::shared_ptr<MTextureResource>& pTextureResource)
{
    static auto onResourceChangedFunction = [this]() {
        SetDirty();
        return true;
    };

    if (m_TextureRef.GetResource<MTextureResource>() == pTextureResource) { return; }

    m_TextureRef.SetResource(pTextureResource);
    m_TextureRef.SetResChangedCallback(onResourceChangedFunction);
    SetDirty();
}

MTexturePtr MTextureResourceParam::GetTexture()
{
    if (auto pTextureResource = m_TextureRef.GetResource<MTextureResource>())
    {
        return pTextureResource->GetTextureTemplate();
    }

    return MShaderTextureParam::GetTexture();
}

std::unique_ptr<MShaderTextureParam> MTextureResourceParam::Clone() const
{
    return std::make_unique<MTextureResourceParam>(*this);
}

void MShaderProgram::BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderType& eType)
{
    uint32_t bitType = 1 << static_cast<size_t>(eType);
    for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
    {
        std::shared_ptr<MShaderParameterSet>& pPropertyTemplate = pBuffer->m_shaderSets[i];
        std::shared_ptr<MShaderParameterSet>& pProgramProperty  = m_shaderSets[i];

        for (const auto& pBufferParam: pPropertyTemplate->m_uniforms)
        {
            if (const auto& pSelfParam = pProgramProperty->FindConstantParam(pBufferParam.get()))
            {
                pSelfParam->eShaderType |= bitType;
                pSelfParam->var = MVariant::Clone(pBufferParam->var);
                pSelfParam->SetDirty();
            }
            else
            {
                auto pParam         = std::make_unique<MShaderUniformParam>(*pBufferParam);
                pParam->eShaderType = bitType;
                pProgramProperty->AppendConstantParam(std::move(pParam));
            }
        }

        for (const auto& pBufferParam: pPropertyTemplate->m_textures)
        {
            if (const auto& pSelfParam = pProgramProperty->FindTextureParam(pBufferParam.get()))
            {
                pSelfParam->eShaderType |= bitType;
            }
            else
            {
                auto pParam         = std::make_unique<MTextureResourceParam>(*pBufferParam);
                pParam->eShaderType = bitType;
                pProgramProperty->AppendTextureParam(std::move(pParam));
            }
        }

        for (const auto& pBufferParam: pPropertyTemplate->m_samplers)
        {
            if (const auto& pSelfParam = pProgramProperty->FindSampleParam(pBufferParam.get()))
            {
                pSelfParam->eShaderType |= bitType;
            }
            else
            {
                auto pParam         = std::make_unique<MShaderSamplerParam>(*pBufferParam);
                pParam->eShaderType = bitType;
                pProgramProperty->AppendSampleParam(std::move(pParam));
            }
        }

        for (const auto& pBufferParam: pPropertyTemplate->m_storages)
        {
            if (const auto& pSelfParam = pProgramProperty->FindStorageParam(pBufferParam.get()))
            {
                pSelfParam->eShaderType |= bitType;
            }
            else
            {
                auto pParam         = std::make_unique<MShaderStorageParam>(*pBufferParam);
                pParam->eShaderType = bitType;
                pProgramProperty->AppendStorageParam(std::move(pParam));
            }
        }
    }
}

void MShaderProgram::UnbindShaderBuffer(const MEShaderType& eType, MIDevice* device)
{
    uint32_t bitType = 1 << static_cast<size_t>(eType);
    for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
    {
        if (std::shared_ptr<MShaderParameterSet> pProgramProperty = m_shaderSets[i])
        {
            auto&& vConstantParams = pProgramProperty->RemoveConstantParam(bitType);
            pProgramProperty->RemoveTextureParam(bitType);
            pProgramProperty->RemoveSampleParam(bitType);
            pProgramProperty->RemoveStorageParam(bitType);

            for (const auto& pParam: vConstantParams) { device->DestroyShaderParamBuffer(pParam.get()); }
        }
    }
}
