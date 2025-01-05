/**
 * @File         MIndirectIndexRenderable
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
class MORTY_API MIndirectIndexRenderable : public IRenderable
{
public:
    void SetMaterial(const MMaterialTemplatePtr& pMaterial) { m_material = pMaterial; }

    void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
    {
        m_propertyAdapter = vAdapter;
    }

    void SetIndirectIndexBuffer(const MBuffer* pBuffer) { m_buffer = pBuffer; }

    void SetMeshBuffer(const std::shared_ptr<MMeshBufferAdapter>& pMeshBuffer) { m_meshBuffer = pMeshBuffer; }

    void Render(MRenderPassCmd* pCommand) override;

    //override to use other material.
    virtual const MMaterialTemplatePtr& GetMaterial() const { return m_material; }

    virtual const MBuffer*              GetIndirectBuffer() const { return m_buffer; }

private:
    std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_propertyAdapter;
    std::shared_ptr<MMeshBufferAdapter>                 m_meshBuffer = nullptr;
    MMaterialTemplatePtr                                m_material   = nullptr;
    const MBuffer*                                      m_buffer     = nullptr;
};

}// namespace morty