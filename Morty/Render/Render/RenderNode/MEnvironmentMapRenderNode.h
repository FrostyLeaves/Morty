/**
 * @File         MEnvironmentMapRenderNode
 * 
 * @Created      2022-01-19 09:24:03
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Material/MMaterial.h"
#include "Object/MObject.h"

#include "RHI/MRenderPass.h"

#include "Resource/MResource.h"

namespace morty
{

class MMeshResource;
class IRenderCommand;
class MSkyBoxComponent;
class MORTY_API MEnvironmentMapRenderNode : public MObject
{
    MORTY_CLASS(MEnvironmentMapRenderNode)
public:
    MEnvironmentMapRenderNode();

    ~MEnvironmentMapRenderNode() override = default;

public:
    void OnCreated() override;

    void OnDelete() override;

public:
    void                       MarkUpdateEnvironment();

    void                       RenderEnvironment(IRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

    std::shared_ptr<MResource> GetDiffuseOutputTexture() const;

protected:
    void RenderDiffuse(IRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

    void RenderSpecular(IRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

protected:
    void InitializeResource();

    void ReleaseResource();

    void InitializeMaterial();

    void ReleaseMaterial();

    void InitializeRenderPass();

    void ReleaseRenderPass();

private:
    bool                                    m_updateNextFrame;

    std::shared_ptr<MMaterial>              m_DiffuseMaterial;
    std::vector<std::shared_ptr<MMaterial>> m_specularMaterial;

    std::shared_ptr<MMeshResource>          m_cubeMesh;

    MResourceRef                            m_DiffuseEnvironmentMap;
    MResourceRef                            m_SpecularEnvironmentMap;

    MRenderPass                             m_DiffuseRenderPass;
    std::vector<MRenderPass>                m_specularRenderPass;
};

}// namespace morty