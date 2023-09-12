/**
 * @File         MIndexedIndirectRenderable
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

class IMaterialFilter;
class MScene;
class MInstanceCulling;
class MMaterialBatchGroup;
class MORTY_API MIndexedIndirectRenderable : public IRenderable
{
public:

	void SetScene(MScene* pScene);

	void SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter);
	void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter);
	void SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCulling);
	void Render(MIRenderCommand* pCommand) override;

	//override to use other material.
	virtual const std::shared_ptr<MMaterial>& GetMaterial(const MMaterialCullingGroup& group) const;

private:

	MScene* m_pScene= nullptr;
	std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_vFramePropertyAdapter;
	std::shared_ptr<MInstanceCulling> m_pCullingAdapter = nullptr;
	std::shared_ptr<IMaterialFilter> pMaterialFilter = nullptr;
};
