/**
 * @File         MShaderPropertyBlock
 * 
 * @Created      2020-07-20 14:26:56
 *
 * @Author       DoubleYe
**/

#pragma once

#include <utility>

#include "Utility/MGlobal.h"
#include "Shader/IShaderProgram.h"
#include "Shader/MShaderProgram.h"

namespace morty
{
class MIDevice;

class MORTY_API MShaderPropertyBlock final
{
public:
    friend class MShaderProgram;

    explicit MShaderPropertyBlock();

    ~MShaderPropertyBlock() = default;

    explicit MShaderPropertyBlock(IShaderProgram* pShaderProgram, const uint32_t& unKey);

    MShaderPropertyBlock(const MShaderPropertyBlock& other);

    const MShaderPropertyBlock& operator=(const MShaderPropertyBlock& other) = delete;

public:
    MShaderConstantParam*        FindConstantParam(const MStringId& strParamName);

    MShaderStorageParam*         FindStorageParam(const MStringId& strParamName);

    MShaderTextureParam*         FindTextureParam(const MStringId& strParamName);


    template<typename TYPE> bool SetValue(const MStringId& strName, const TYPE& value);

    bool                         SetTexture(const MStringId& strName, const MTexturePtr& pTexture);

    bool                         HasValue(const uint32_t& unBinding, const uint32_t& unSet);

public:
    MShaderConstantParam* FindConstantParam(const MShaderConstantParam* param)
    {
        return FindShaderParam(m_params, param);
    }

    void AppendConstantParam(std::unique_ptr<MShaderConstantParam> pParam)
    {
        return AppendShaderParam(m_params, std::move(pParam));
    }

    std::vector<std::unique_ptr<MShaderConstantParam>> RemoveConstantParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderConstantParam>(m_params, eShaderType);
    }

    MShaderTextureParam* FindTextureParam(const MShaderTextureParam* param)
    {
        return FindShaderParam(m_textures, param);
    }

    void AppendTextureParam(std::unique_ptr<MShaderTextureParam> pParam)
    {
        return AppendShaderParam(m_textures, std::move(pParam));
    }

    std::vector<std::unique_ptr<MShaderTextureParam>> RemoveTextureParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderTextureParam>(m_textures, eShaderType);
    }

    MShaderSampleParam* FindSampleParam(const MShaderSampleParam* param) { return FindShaderParam(m_samples, param); }

    void                AppendSampleParam(std::unique_ptr<MShaderSampleParam> pParam)
    {
        return AppendShaderParam(m_samples, std::move(pParam));
    }

    std::vector<std::unique_ptr<MShaderSampleParam>> RemoveSampleParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderSampleParam>(m_samples, eShaderType);
    }

    MShaderStorageParam* FindStorageParam(const MShaderStorageParam* param)
    {
        return FindShaderParam(m_storages, param);
    }

    void AppendStorageParam(std::unique_ptr<MShaderStorageParam> pParam)
    {
        return AppendShaderParam(m_storages, std::move(pParam));
    }

    std::vector<std::unique_ptr<MShaderStorageParam>> RemoveStorageParam(const uint32_t& eShaderType)
    {
        return RemoveShaderParam<MShaderStorageParam>(m_storages, eShaderType);
    }


    void                                                GenerateBuffer(MIDevice* pDevice);

    void                                                DestroyBuffer(MIDevice* pDevice);

    [[nodiscard]] std::shared_ptr<MShaderPropertyBlock> Clone() const;

    [[nodiscard]] std::shared_ptr<MShaderPropertyBlock> GetShared() const;

    [[nodiscard]] IShaderProgram*                       GetShaderProgram() const { return m_shaderProgram; }

    [[nodiscard]] const std::vector<std::unique_ptr<MShaderConstantParam>>& GetConstantParams() const
    {
        return m_params;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<MShaderTextureParam>>& GetTextureParams() const
    {
        return m_textures;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<MShaderSampleParam>>&  GetSampleParams() const { return m_samples; }

    [[nodiscard]] const std::vector<std::unique_ptr<MShaderStorageParam>>& GetStorageParams() const
    {
        return m_storages;
    }

private:
    std::vector<std::unique_ptr<MShaderConstantParam>> m_params;
    std::vector<std::unique_ptr<MShaderTextureParam>>  m_textures;
    std::vector<std::unique_ptr<MShaderSampleParam>>   m_samples;
    std::vector<std::unique_ptr<MShaderStorageParam>>  m_storages;

public:
    uint32_t        m_unKey;
    IShaderProgram* m_shaderProgram;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkDescriptorSet m_vkDescriptorSet = VK_NULL_HANDLE;
#endif

protected:
    template<typename ParamType>
    ParamType* FindShaderParam(std::vector<std::unique_ptr<ParamType>>& vVector, const ParamType* pParam);

    template<typename ParamType>
    void AppendShaderParam(std::vector<std::unique_ptr<ParamType>>& vVector, std::unique_ptr<ParamType> pParam);

    template<typename ParamType> std::vector<std::unique_ptr<ParamType>>
    RemoveShaderParam(std::vector<std::unique_ptr<ParamType>>& vVector, const uint32_t& eShaderType);
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
    for (std::unique_ptr<MShaderConstantParam>& pParam: m_params)
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

    return false;
}

template<typename ParamType>
ParamType* MShaderPropertyBlock::FindShaderParam(std::vector<std::unique_ptr<ParamType>>& from, const ParamType* pParam)
{
    size_t size = from.size();
    for (size_t i = 0; i < size; ++i)
    {
        std::unique_ptr<ParamType>& param = from[i];

        if (pParam->unSet == param->unSet && pParam->unBinding == param->unBinding) { return from[i].get(); }
    }

    return nullptr;
}

template<typename ParamType> void MShaderPropertyBlock::AppendShaderParam(
        std::vector<std::unique_ptr<ParamType>>& vVector,
        std::unique_ptr<ParamType>               pParam
)
{
    vVector.push_back(std::move(pParam));
}

template<typename ParamType> std::vector<std::unique_ptr<ParamType>>
MShaderPropertyBlock::RemoveShaderParam(std::vector<std::unique_ptr<ParamType>>& vVector, const uint32_t& eShaderType)
{
    std::vector<std::unique_ptr<ParamType>> vResult;

    for (auto iter = vVector.begin(); iter != vVector.end();)
    {
        std::unique_ptr<ParamType>& param = *iter;

        if (param->eShaderType & eShaderType) { param->eShaderType = param->eShaderType ^ eShaderType; }

        if (0 == param->eShaderType)
        {
            vResult.push_back(std::move(param));
            iter = vVector.erase(iter);
        }
        else { ++iter; }
    }

    return vResult;
}
}// namespace morty
