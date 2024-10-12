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

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

namespace morty
{

class IMaterialFilter;
class MScene;
class MInstanceCulling;
class MMaterialBatchGroup;
class MORTY_API MIndirectIndexRenderable : public IRenderable
{
public:
    void SetMaterial(const std::shared_ptr<MMaterial>& pMaterial) { m_material = pMaterial; }

    void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
    {
        m_propertyAdapter = vAdapter;
    }

    void SetIndirectIndexBuffer(const MBuffer* pBuffer) { m_buffer = pBuffer; }

    void SetMeshBuffer(const std::shared_ptr<MMeshBufferAdapter>& pMeshBuffer) { m_meshBuffer = pMeshBuffer; }

    void Render(MIRenderCommand* pCommand) override;

    //override to use other material.
    virtual const std::shared_ptr<MMaterial>& GetMaterial() const { return m_material; }

    virtual const MBuffer*                    GetIndirectBuffer() const { return m_buffer; }

private:
    std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_propertyAdapter;
    std::shared_ptr<MMeshBufferAdapter>                 m_meshBuffer = nullptr;
    std::shared_ptr<MMaterial>                          m_material   = nullptr;
    const MBuffer*                                      m_buffer     = nullptr;
};

}// namespace morty