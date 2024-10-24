/**
 * @File         MShaderPropertyBlock
 * 
 * @Created      2020-07-20 14:26:56
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Shader/MShaderParam.h"

namespace morty
{

class MIDevice;
class MShaderProgram;
class MORTY_API MShaderPropertyBlock final
{
public:
    explicit MShaderPropertyBlock();

    ~MShaderPropertyBlock() = default;

    explicit MShaderPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey);

    explicit MShaderPropertyBlock(const MShaderPropertyBlock& other);

    const MShaderPropertyBlock& operator=(const MShaderPropertyBlock& other) = delete;

    static std::shared_ptr<MShaderPropertyBlock>
    MakeShared(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey);

    static std::shared_ptr<MShaderPropertyBlock> MakeShared(const std::shared_ptr<MShaderPropertyBlock>& other);

public:
    std::shared_ptr<MShaderConstantParam> FindConstantParam(const MStringId& strParamName);

    std::shared_ptr<MShaderStorageParam>  FindStorageParam(const MStringId& strParamName);

    std::shared_ptr<MShaderTextureParam>  FindTextureParam(const MStringId& strParamName);


    template<typename TYPE> bool          SetValue(const MStringId& strName, const TYPE& value);

    bool                                  SetTexture(const MStringId& strName, MTexturePtr pTexture);

    bool                                  HasValue(const uint32_t& unBinding, const uint32_t& unSet);

public:
    std::shared_ptr<MShaderConstantParam> FindConstantParam(std::shared_ptr<const MShaderConstantParam> pParam)
    {
        return FindShaderParam(m_params, pParam);
    }

    void AppendConstantParam(std::shared_ptr<MShaderConstantParam> pParam, const uint32_t& eShaderType)
    {
        return AppendShaderParam(m_params, pParam, eShaderType);
    }

    std::vector<std::shared_ptr<MShaderConstantParam>> RemoveConstantParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderConstantParam>(m_params, eShaderType);
    }

    std::shared_ptr<MShaderTextureParam> FindTextureParam(std::shared_ptr<const MShaderTextureParam> pParam)
    {
        return FindShaderParam(m_textures, pParam);
    }

    void AppendTextureParam(std::shared_ptr<MShaderTextureParam> pParam, const uint32_t& eShaderType)
    {
        return AppendShaderParam(m_textures, pParam, eShaderType);
    }

    std::vector<std::shared_ptr<MShaderTextureParam>> RemoveTextureParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderTextureParam>(m_textures, eShaderType);
    }

    std::shared_ptr<MShaderSampleParam> FindSampleParam(std::shared_ptr<const MShaderSampleParam> pParam)
    {
        return FindShaderParam(m_samples, pParam);
    }

    void AppendSampleParam(std::shared_ptr<MShaderSampleParam> pParam, const uint32_t& eShaderType)
    {
        return AppendShaderParam(m_samples, pParam, eShaderType);
    }

    std::vector<std::shared_ptr<MShaderSampleParam>> RemoveSampleParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderSampleParam>(m_samples, eShaderType);
    }

    std::shared_ptr<MShaderStorageParam> FindStorageParam(std::shared_ptr<const MShaderStorageParam> pParam)
    {
        return FindShaderParam(m_storages, pParam);
    }

    void AppendStorageParam(std::shared_ptr<MShaderStorageParam> pParam, const uint32_t& eShaderType)
    {
        return AppendShaderParam(m_storages, pParam, eShaderType);
    }

    std::vector<std::shared_ptr<MShaderStorageParam>> RemoveStorageParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderStorageParam>(m_storages, eShaderType);
    }


    void                                  GenerateBuffer(MIDevice* pDevice);

    void                                  DestroyBuffer(MIDevice* pDevice);

    std::shared_ptr<MShaderPropertyBlock> Clone() const;

    std::shared_ptr<MShaderPropertyBlock> GetShared() const;

    std::shared_ptr<MShaderProgram>       GetShaderProgram() const { return m_shaderProgram.lock(); }

public:
    std::vector<std::shared_ptr<MShaderConstantParam>> m_params;
    std::vector<std::shared_ptr<MShaderTextureParam>>  m_textures;
    std::vector<std::shared_ptr<MShaderSampleParam>>   m_samples;
    std::vector<std::shared_ptr<MShaderStorageParam>>  m_storages;

public:
    uint32_t                            m_unKey;
    std::weak_ptr<MShaderProgram>       m_shaderProgram;
    std::weak_ptr<MShaderPropertyBlock> m_selfPointer;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkDescriptorSet m_vkDescriptorSet = VK_NULL_HANDLE;
#endif

protected:
    template<typename ParamType> std::shared_ptr<ParamType>
    FindShaderParam(std::vector<std::shared_ptr<ParamType>>& vVector, const std::shared_ptr<const ParamType> pParam);

    template<typename ParamType> void AppendShaderParam(
            std::vector<std::shared_ptr<ParamType>>& vVector,
            std::shared_ptr<ParamType>               pParam,
            const uint32_t&                          eShaderType
    );

    template<typename ParamType> std::vector<std::shared_ptr<ParamType>>
    RemoveShaderParam(std::vector<std::shared_ptr<ParamType>>& vVector, const uint32_t& eShaderType);
};

template<typename TYPE> inline bool SetValueRecursive(MVariant& variant, const MStringId& strName, const TYPE& value)
{
    if (variant.IsType<MVariantStruct>())
    {
        MVariant& findResult = variant.GetValue<MVariantStruct>().FindVariant(strName);
        if (findResult.IsType<TYPE>())
        {
            findResult.SetValue(value);
            return true;
        }

        for (auto member: variant.GetValue<MVariantStruct>().GetMember())
        {
            MVariant& child = member.second;
            if (child.IsType<MVariantStruct>())
            {
                if (SetValueRecursive(child, strName, value)) { return true; }
            }
        }
    }

    return false;
}

template<typename TYPE> inline bool MShaderPropertyBlock::SetValue(const MStringId& strName, const TYPE& value)
{
    for (std::shared_ptr<MShaderConstantParam>& pParam: m_params)
    {
        if (pParam->var.IsType<TYPE>() && pParam->strName == strName)
        {
            pParam->var.SetValue(value);
            pParam->SetDirty();
            return true;
        }
        else if (pParam->var.GetType() == MEVariantType::EStruct)
        {
            if (SetValueRecursive(pParam->var, strName, value))
            {
                pParam->SetDirty();
                return true;
            }
        }
    }

    MORTY_ASSERT(false);
    return false;
}

template<typename ParamType> std::shared_ptr<ParamType> MShaderPropertyBlock::FindShaderParam(
        std::vector<std::shared_ptr<ParamType>>& vVector,
        const std::shared_ptr<const ParamType>   pParam
)
{
    size_t unNum = vVector.size();
    for (size_t i = 0; i < unNum; ++i)
    {
        std::shared_ptr<ParamType> param = vVector[i];

        if (pParam->unSet == param->unSet && pParam->unBinding == param->unBinding) { return vVector[i]; }
    }

    return nullptr;
}

template<typename ParamType> void MShaderPropertyBlock::AppendShaderParam(
        std::vector<std::shared_ptr<ParamType>>& vVector,
        std::shared_ptr<ParamType>               pParam,
        const uint32_t&                          eShaderType
)
{
    MORTY_UNUSED(eShaderType);

    vVector.push_back(pParam);
}

template<typename ParamType> std::vector<std::shared_ptr<ParamType>>
MShaderPropertyBlock::RemoveShaderParam(std::vector<std::shared_ptr<ParamType>>& vVector, const uint32_t& eShaderType)
{
    std::vector<std::shared_ptr<ParamType>> vResult;

    for (auto iter = vVector.begin(); iter != vVector.end();)
    {
        std::shared_ptr<ParamType> param = *iter;

        if (param->eShaderType & eShaderType) { param->eShaderType = param->eShaderType ^ eShaderType; }

        if (0 == param->eShaderType)
        {
            vResult.push_back(*iter);
            iter = vVector.erase(iter);
        }
        else { ++iter; }
    }

    return vResult;
}

}// namespace morty