/**
 * @File         MComputerDispatcher
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

#include "Shader/MShaderBuffer.h"
#include "Shader/MShaderMacro.h"
#include "Shader/MShaderParameterSet.h"
#include "Shader/MShaderProgram.h"


namespace morty
{

class MORTY_API MComputeDispatcher : public MObject
{
public:
    MORTY_CLASS(MComputeDispatcher)

    MComputeDispatcher() = default;

    virtual ~MComputeDispatcher() = default;

public:
    bool LoadComputeShader(const std::shared_ptr<MResource>& resource, const MStringId& entryName);

    bool LoadComputeShader(const MString& strResource, const MStringId& entryName);

public:
    std::shared_ptr<MResource> GetComputeShaderResource() { return m_shaderProgram->GetShaderResource(); }

    std::array<std::shared_ptr<MShaderParameterSet>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParameterSets()
    {
        return m_shaderProgram->GetShaderParameterSets();
    }

    std::shared_ptr<MShaderParameterSet> GetShaderParameterSet(size_t nSetIdx)
    {
        return GetShaderParameterSets()[nSetIdx];
    }

    MShader*                      GetComputeShader();

    const MShaderMacro&           GetShaderMacro() const { return m_shaderProgram->GetShaderMacro(); }

    [[nodiscard]] MShaderProgram* GetShaderProgram() const { return m_shaderProgram.get(); }

    void                          SetDispatcherID(const uint32_t& nID) { m_unDispatcherID = nID; }

    uint32_t                      GetDispatcherID() const { return m_unDispatcherID; }

public:
    virtual void OnCreated() override;

    virtual void OnDelete() override;


private:
    std::unique_ptr<MShaderProgram> m_shaderProgram  = nullptr;
    uint32_t                        m_unDispatcherID = 0;

public:
    void GeneratePipeline();

    void DestroyPipeline();


#if RENDER_GRAPHICS == MORTY_VULKAN

    VkPipeline m_vkPipeline;

#endif

#if MORTY_DEBUG

    const char* GetDebugName() const { return m_strDebugName.c_str(); }

    MString     m_strDebugName;
#endif
};

}// namespace morty