/**
 * @File         MMaterialTemplate
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Resource/MTextureResource.h"

#include "MMaterialPass.h"
#include "Shader/MShaderMacro.h"
#include "Shader/MShaderParameterSet.h"
#include "Shader/MShaderProgram.h"



namespace morty
{

class MShader;
class MShaderResource;
class MORTY_API MMaterialTemplate : public MResource
{
public:
    MORTY_CLASS(MMaterialTemplate)

    MMaterialTemplate() = default;

    ~MMaterialTemplate() override = default;

    bool                       LoadShader(const std::shared_ptr<MResource>& pResource);
    bool                       LoadShader(const MString& strResource);
    std::shared_ptr<MResource> GetShaderResource() const { return m_shaderResource.GetResource(); }

    MMaterialPass* SetPass(const MStringId& passName, const MStringId& vsEntryName, const MStringId& psEntryName);
    [[nodiscard]] MMaterialPass* GetPass(const MStringId& passName) const;
    [[nodiscard]] const std::unordered_map<MStringId, std::unique_ptr<MMaterialPass>>& GetPasses() const
    {
        return m_passes;
    }

    [[nodiscard]] MMaterialPass* GetDefaultPass() const;

    void                         SetShaderMacro(const MShaderMacro& macro)
    {
        m_shaderMacro = macro;
        SetDirty();
    }
    [[nodiscard]] const MShaderMacro&    GetShaderMacro() const { return m_shaderMacro; }

    std::shared_ptr<MShaderParameterSet> CreateParameterSet(size_t setIdx) const;

    [[nodiscard]] MHashCode              GetHashCode() const;

public:
    void OnCreated() override;

    void OnDelete() override;

    void SetDirty() { m_dirty = true; }

protected:
    bool                                                          m_dirty       = true;
    MShaderMacro                                                  m_shaderMacro = {};
    MResourceRef                                                  m_shaderResource;// Shader resources
    std::unordered_map<MStringId, std::unique_ptr<MMaterialPass>> m_passes;        // Material passes
};

}// namespace morty