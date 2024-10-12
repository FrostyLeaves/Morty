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
#include "Shader/MShaderProgram.h"
#include "Shader/MShaderPropertyBlock.h"

namespace morty
{

class MORTY_API MComputeDispatcher : public MObject
{
public:
    MORTY_CLASS(MComputeDispatcher)

    MComputeDispatcher() = default;

    virtual ~MComputeDispatcher() = default;

public:
    bool LoadComputeShader(std::shared_ptr<MResource> pResource);

    bool LoadComputeShader(const MString& strResource);

public:
    std::shared_ptr<MResource> GetComputeShaderResource()
    {
        return m_shaderProgram->GetShaderResource(MEShaderType::ECompute);
    }

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks()
    {
        return m_shaderProgram->GetShaderPropertyBlocks();
    }

    std::shared_ptr<MShaderPropertyBlock> GetShaderPropertyBlock(size_t nSetIdx)
    {
        return GetShaderPropertyBlocks()[nSetIdx];
    }

    MShader*                        GetComputeShader();

    MShaderMacro&                   GetShaderMacro() { return m_shaderProgram->GetShaderMacro(); }

    std::shared_ptr<MShaderProgram> GetShaderProgram() const { return m_shaderProgram; }

    void                            SetDispatcherID(const uint32_t& nID) { m_unDispatcherID = nID; }

    uint32_t                        GetDispatcherID() const { return m_unDispatcherID; }

public:
    virtual void OnCreated() override;

    virtual void OnDelete() override;


private:
    std::shared_ptr<MShaderProgram> m_shaderProgram  = nullptr;
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