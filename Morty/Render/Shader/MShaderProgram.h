/**
 * @File         MShaderProgram
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

#include "IShaderProgram.h"
#include "MShaderBuffer.h"
#include "MShaderMacro.h"
#include "Shader/MShader.h"
#include "Shader/MShaderParam.h"

namespace morty
{

struct MORTY_API MTextureResourceParam : public MShaderTextureParam {
public:
                MTextureResourceParam();

    explicit    MTextureResourceParam(const MShaderTextureParam& param);
    void        SetTexture(const std::shared_ptr<MTextureResource>& pTextureResource);
    void        SetTexture(MTexturePtr pTexture) override;
    MTexturePtr GetTexture() override;


    [[nodiscard]] std::unique_ptr<MShaderTextureParam> Clone() const override;

    [[nodiscard]] std::shared_ptr<MTextureResource>    GetTextureResource() const
    {
        return m_TextureRef.GetResource<MTextureResource>();
    }

private:
    MResourceRef m_TextureRef;
};

class MShader;
class MShaderResource;

class MORTY_API MShaderProgram : public IShaderProgram
{

public:
    MORTY_CLASS(MShaderProgram)

    explicit MShaderProgram() = default;
    explicit MShaderProgram(
            MEngine*                          pEngine,
            EUsage                            usage,
            const std::shared_ptr<MResource>& shader,
            const MShaderMacro&               macro,
            const MEntryNames&                entryNames,
            uint8_t                           shaderMask
    );

    ~                                        MShaderProgram() override;

    [[nodiscard]] std::shared_ptr<MResource> GetShaderResource() const override
    {
        return m_shaderResource.GetResource();
    }

    [[nodiscard]] MShader*  GetShader(MEShaderType eType) override;
    MShaderMacro&           GetShaderMacro() override { return m_shaderMacro; }
    [[nodiscard]] MStringId GetEntryName(MEShaderType shaderTYpe) const override;

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>&
                            GetShaderPropertyBlocks() override;

    [[nodiscard]] MHashCode GetHashCode() const override;
    [[nodiscard]] bool      IsValid() const override;

private:
    void        InitializeShaderPropertyBlock();
    bool        LoadShader(const std::shared_ptr<MResource>& pResource);

    void        UnloadShader();

    static void CopyShaderParams(
            MEngine*                                           pEngine,
            const std::shared_ptr<MShaderPropertyBlock>&       target,
            const std::shared_ptr<const MShaderPropertyBlock>& source
    );

    std::shared_ptr<MShaderPropertyBlock> AllocShaderPropertyBlock(size_t nSetIdx);

    void ReleaseShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock);

protected:
    [[nodiscard]] MEngine* GetEngine() const { return m_engine; }

    void                   CompileShaderIfNeed();

    void                   BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderType& eType);
    void                   UnbindShaderBuffer(const MEShaderType& eType, MIDevice* device);

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_shaderSets;
    std::set<std::shared_ptr<MShaderPropertyBlock>> m_shaderPropertyBlockInstance;


    enum ShaderState
    {
        Unknow = 0,
        Compiled,
        Failed
    };

    struct CompiledShader {
        MShader*    pShader    = nullptr;
        int         nShaderIdx = 0;
        ShaderState state      = ShaderState::Unknow;
    };

    MResourceRef                                                 m_shaderResource;
    MShaderMacro                                                 m_shaderMacro;
    MEngine*                                                     m_engine = nullptr;
    EUsage                                                       m_usage  = EUsage::EUnknow;
    MEntryNames                                                  m_entryNames;
    uint8_t                                                      m_shaderMask;

    std::array<CompiledShader, (size_t) MEShaderType::TOTAL_NUM> m_compiledShaders;
};

}// namespace morty