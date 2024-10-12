#include "MShaderProgram.h"

#include "Engine/MEngine.h"
#include "MShaderBuffer.h"
#include "RHI/Abstract/MIDevice.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Shader/MShader.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShaderProgram, MTypeClass)

MShaderProgram::MShaderProgram(MEngine* pEngine, EUsage usage)
    : MTypeClass()
    , m_ShaderMacro()
    , m_engine(pEngine)
    , m_usage(usage)
{}

void MShaderProgram::InitializeShaderPropertyBlock()
{
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] =
            MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] =
            MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] =
            MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_OTHER] =
            MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_OTHER);
}

std::shared_ptr<MShaderProgram> MShaderProgram::MakeShared(MEngine* pEngine, EUsage usage)
{
    std::shared_ptr<MShaderProgram> pResult = std::make_shared<MShaderProgram>(pEngine, usage);
    pResult->m_selfPointer                  = pResult;
    pResult->InitializeShaderPropertyBlock();
    return pResult;
}


MShaderProgram::~MShaderProgram() {}

bool MShaderProgram::LoadShader(std::shared_ptr<MResource> pResource)
{
    if (std::shared_ptr<MShaderResource> pShaderResource = MTypeClass::DynamicCast<MShaderResource>(pResource))
    {
        auto eShaderType = pShaderResource->GetShaderType();
        auto LoadFunc    = [this, eShaderType]() {
            MShaderDesc&     desc            = m_shaders[static_cast<size_t>(eShaderType)];
            MShaderResource* pShaderResource = desc.resource.GetResource()->template DynamicCast<MShaderResource>();
            if (nullptr == pShaderResource) { return false; }

            desc.nShaderIdx = pShaderResource->FindShaderByMacroParam(m_ShaderMacro);
            desc.pShader    = pShaderResource->GetShaderByIndex(desc.nShaderIdx);
            if (desc.pShader && nullptr == desc.pShader->GetBuffer())
            {
                MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
                if (!desc.pShader->CompileShader(pRenderSystem->GetDevice()))
                {
                    MORTY_ASSERT(false);
                    desc.pShader = nullptr;
                    return false;
                }
            }
            UnbindShaderBuffer(eShaderType);
            BindShaderBuffer(desc.pShader->GetBuffer(), eShaderType);
            return true;
        };

        MShaderDesc& desc = m_shaders[static_cast<size_t>(eShaderType)];
        desc.resource.SetResource(pResource);
        desc.resource.SetResChangedCallback(LoadFunc);

        LoadFunc();
        return true;
    }

    return false;
}

void MShaderProgram::SetShaderMacro(const MShaderMacro& macro)
{
    m_ShaderMacro = macro;

    for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
    {
        if (auto pResource = GetShaderResource(MEShaderType(nIdx))) { LoadShader(pResource); }
    }
}

void MShaderProgram::BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderType& eType)
{
    uint32_t bitType = 1 << static_cast<size_t>(eType);
    for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
    {
        std::shared_ptr<MShaderPropertyBlock>& pPropertyTemplate = pBuffer->m_shaderSets[i];
        std::shared_ptr<MShaderPropertyBlock>& pProgramProperty  = m_shaderSets[i];

        for (const std::shared_ptr<MShaderConstantParam>& pBufferParam: pPropertyTemplate->m_params)
        {
            if (std::shared_ptr<MShaderConstantParam> pSelfParam = pProgramProperty->FindConstantParam(pBufferParam))
            {
                pSelfParam->eShaderType |= bitType;
                pSelfParam->var = MVariant::Clone(pBufferParam->var);
                pSelfParam->SetDirty();
            }
            else
            {
                std::shared_ptr<MShaderConstantParam> pParam = std::make_shared<MShaderConstantParam>(*pBufferParam);
                pParam->eShaderType                          = bitType;
                pProgramProperty->AppendConstantParam(pParam, bitType);
            }
        }

        for (const std::shared_ptr<MShaderTextureParam>& pBufferParam: pPropertyTemplate->m_textures)
        {
            if (std::shared_ptr<MShaderTextureParam> pSelfParam = pProgramProperty->FindTextureParam(pBufferParam))
            {
                pSelfParam->eShaderType |= bitType;
            }
            else
            {
                std::shared_ptr<MShaderTextureParam> pParam = std::make_shared<MTextureResourceParam>(*pBufferParam);
                pParam->eShaderType                         = bitType;
                pProgramProperty->AppendTextureParam(pParam, bitType);
            }
        }

        for (const std::shared_ptr<MShaderSampleParam>& pBufferParam: pPropertyTemplate->m_samples)
        {
            if (std::shared_ptr<MShaderSampleParam> pSelfParam = pProgramProperty->FindSampleParam(pBufferParam))
            {
                pSelfParam->eShaderType |= bitType;
            }
            else
            {
                std::shared_ptr<MShaderSampleParam> pParam = std::make_shared<MShaderSampleParam>(*pBufferParam);
                pParam->eShaderType                        = bitType;
                pProgramProperty->AppendSampleParam(pParam, bitType);
            }
        }

        for (const std::shared_ptr<MShaderStorageParam>& pBufferParam: pPropertyTemplate->m_storages)
        {
            if (std::shared_ptr<MShaderStorageParam> pSelfParam = pProgramProperty->FindStorageParam(pBufferParam))
            {
                pSelfParam->eShaderType |= bitType;
            }
            else
            {
                std::shared_ptr<MShaderStorageParam> pParam = std::make_shared<MShaderStorageParam>(*pBufferParam);
                pParam->eShaderType                         = bitType;
                pProgramProperty->AppendStorageParam(pParam, bitType);
            }
        }
    }
}

void MShaderProgram::UnbindShaderBuffer(const MEShaderType& eType)
{
    uint32_t       bitType       = 1 << static_cast<size_t>(eType);
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
    {
        if (std::shared_ptr<MShaderPropertyBlock> pProgramProperty = m_shaderSets[i])
        {
            std::vector<std::shared_ptr<MShaderConstantParam>>&& vConstantParams =
                    pProgramProperty->RemoveConstantParam(bitType);
            pProgramProperty->RemoveTextureParam(bitType);
            pProgramProperty->RemoveSampleParam(bitType);
            pProgramProperty->RemoveStorageParam(bitType);

            for (std::shared_ptr<MShaderConstantParam>& pParam: vConstantParams)
            {
                pRenderSystem->GetDevice()->DestroyShaderParamBuffer(pParam);
            }
        }
    }
}

void MShaderProgram::ClearShader()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    for (size_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
    {
        m_shaderSets[i]->DestroyBuffer(pRenderSystem->GetDevice());
        m_shaderSets[i] = nullptr;
    }

    for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
    {
        m_shaders[nIdx].resource.SetResource(nullptr);
        m_shaders[nIdx].pShader    = nullptr;
        m_shaders[nIdx].nShaderIdx = 0;
    }
}

void MShaderProgram::CopyShaderParams(
        MEngine*                                           pEngine,
        const std::shared_ptr<MShaderPropertyBlock>&       target,
        const std::shared_ptr<const MShaderPropertyBlock>& source
)
{
    MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

    target->DestroyBuffer(pRenderSystem->GetDevice());

    target->m_params.resize(source->m_params.size());
    for (uint32_t i = 0; i < source->m_params.size(); ++i)
    {
        target->m_params[i] = std::make_shared<MShaderConstantParam>(*source->m_params[i]);
    }

    target->m_textures.resize(source->m_textures.size());
    for (uint32_t i = 0; i < source->m_textures.size(); ++i)
    {
        std::shared_ptr<MTextureResourceParam> pSource =
                std::dynamic_pointer_cast<MTextureResourceParam>(source->m_textures[i]);
        std::shared_ptr<MTextureResourceParam> pParam = std::make_shared<MTextureResourceParam>(*pSource);

        pParam->SetTexture(pSource->GetTextureResource());

        target->m_textures[i] = pParam;
    }

    target->m_samples.resize(source->m_samples.size());
    for (uint32_t i = 0; i < source->m_samples.size(); ++i)
    {
        target->m_samples[i] = std::make_shared<MShaderSampleParam>(*source->m_samples[i]);
    }

    target->m_storages.resize(source->m_storages.size());
    for (uint32_t i = 0; i < source->m_storages.size(); ++i)
    {
        target->m_storages[i] = std::make_shared<MShaderStorageParam>(*source->m_storages[i]);
    }
}

std::shared_ptr<MShaderPropertyBlock> MShaderProgram::AllocShaderPropertyBlock(size_t nSetIdx)
{
    std::shared_ptr<MShaderPropertyBlock> pShaderPropertyBlock = m_shaderSets[nSetIdx]->Clone();
    m_shaderPropertyBlockInstance.insert(pShaderPropertyBlock);

    return pShaderPropertyBlock;
}

void MShaderProgram::ReleaseShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock)
{
    m_shaderPropertyBlockInstance.erase(pShaderPropertyBlock);
}

std::shared_ptr<MShaderProgram> MShaderProgram::GetShared() const { return m_selfPointer.lock(); }

MTextureResourceParam::MTextureResourceParam()
    : MShaderTextureParam()
    , m_TextureRef()
{}

MTextureResourceParam::MTextureResourceParam(const MShaderTextureParam& param)
    : MShaderTextureParam(param)
    , m_TextureRef()
{}

void MTextureResourceParam::SetTexture(std::shared_ptr<MTexture> pTexture)
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

std::shared_ptr<MTexture> MTextureResourceParam::GetTexture()
{
    if (auto pTextureResource = m_TextureRef.GetResource<MTextureResource>())
    {
        return pTextureResource->GetTextureTemplate();
    }

    return MShaderTextureParam::GetTexture();
}

std::shared_ptr<MShaderTextureParam> MTextureResourceParam::Clone() const
{
    return std::make_shared<MTextureResourceParam>(*this);
}
