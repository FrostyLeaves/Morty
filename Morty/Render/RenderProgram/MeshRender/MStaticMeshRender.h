/**
 * @File         MStaticMeshRender
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_STATIC_MESH_RENDER_H_
#define _M_STATIC_MESH_RENDER_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

class MScene;
class MRenderableMaterialGroup;
class MORTY_API MStaticMeshRender : public IRenderable
{
public:

	void SetScene(MScene* pScene);
	void SetRenderableFilter(std::shared_ptr<IRenderableFilter> pRenderableFilter);
	void SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);
	void SetRenderableMaterialGroup(const std::vector<MRenderableMaterialGroup*>& vRenderGroup);

	void Render(MIRenderCommand* pCommand) override;

private:

	MScene* m_pScene= nullptr;
	std::shared_ptr<IRenderableFilter> m_pRenderableFilter = nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
	std::vector<MRenderableMaterialGroup*> m_vRenderGroup;
};


#endif
