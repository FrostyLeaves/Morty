/**
 * @File         MCullingResultRenderable
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"

namespace morty
{

class IMaterialFilter;
class MScene;
class MInstanceCulling;
class MMaterialBatchGroup;
class MORTY_API MCullingResultRenderable : public IRenderable
{
public:
    void SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter);

    void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter);

    void SetMeshBuffer(const std::shared_ptr<MMeshBufferAdapter>& pMeshBuffer) { m_meshBuffer = pMeshBuffer; }

    void SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCulling);

    void Render(MIRenderCommand* pCommand) override;

    //override to use other material.
    virtual std::shared_ptr<MMaterial> GetMaterial(const MMaterialCullingGroup& group) const;

private:
    std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_framePropertyAdapter;
    std::shared_ptr<MInstanceCulling>                   m_cullingAdapter = nullptr;
    std::shared_ptr<MMeshBufferAdapter>                 m_meshBuffer     = nullptr;
    std::shared_ptr<IMaterialFilter>                    pMaterialFilter  = nullptr;
};

}// namespace morty