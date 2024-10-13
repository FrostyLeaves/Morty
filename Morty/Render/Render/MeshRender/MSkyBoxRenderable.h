/**
 * @File         MSkyBoxRenderable
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

class MScene;
class MSkyBoxComponent;
class MMaterialBatchGroup;
class MORTY_API MSkyBoxRenderable : public IRenderable
{
public:
    void SetMesh(MIMesh* pMesh);

    void SetMaterial(const std::shared_ptr<MMaterial>& pMaterial);

    void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter);

    void Render(MIRenderCommand* pCommand) override;

private:
    MIMesh*                                             m_mesh     = nullptr;
    std::shared_ptr<MMaterial>                          m_material = nullptr;
    std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_framePropertyAdapter;
};

}// namespace morty