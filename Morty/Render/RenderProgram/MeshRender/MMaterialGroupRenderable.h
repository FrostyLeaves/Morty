/**
 * @File         MMaterialGroupRenderable
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_MATERIAL_GROUP_RENDERABLE_H_
#define _M_MATERIAL_GROUP_RENDERABLE_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

class IMaterialFilter;
class IMeshInstanceFilter;
class MScene;
class MRenderableMaterialGroup;
class MORTY_API MMaterialGroupRenderable : public IRenderable
{
public:

	void SetScene(MScene* pScene);
	void SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter);
	void SetInstanceFilter(std::shared_ptr<IMeshInstanceFilter> pFilter);
	void SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);
	void SetRenderableMaterialGroup(const std::vector<MRenderableMaterialGroup*>& vRenderGroup);

	void Render(MIRenderCommand* pCommand) override;

private:

	MScene* m_pScene= nullptr;
	std::shared_ptr<IMaterialFilter> m_pMaterialFilter = nullptr;
	std::shared_ptr<IMeshInstanceFilter> m_pInstanceFilter = nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
	std::vector<MRenderableMaterialGroup*> m_vRenderGroup;
};


#endif
