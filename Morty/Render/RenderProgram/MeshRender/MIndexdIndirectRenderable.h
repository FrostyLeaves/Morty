/**
 * @File         MIndexdIndirectRenderable
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_GPU_DRIVEN_RENDER_H_
#define _M_GPU_DRIVEN_RENDER_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

class IMaterialFilter;
class MScene;
class MInstanceCulling;
class MRenderableMaterialGroup;
class MORTY_API MIndexdIndirectRenderable : public IRenderable
{
public:

	void SetScene(MScene* pScene);

	void SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter);
	void SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);
	void SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCulling);
	void Render(MIRenderCommand* pCommand) override;

private:

	MScene* m_pScene= nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
	std::shared_ptr<MInstanceCulling> m_pCullingAdapter = nullptr;
	std::shared_ptr<IMaterialFilter> pMaterialFilter = nullptr;
};

class MORTY_API MComputeDispatcherRender : public IRenderable
{
public:

	void SetComputeDispatcher(const std::shared_ptr<IComputeDispatcherAdapter>& pComputeDispatcher);

	void Render(MIRenderCommand* pCommand) override;

private:
	std::shared_ptr<IComputeDispatcherAdapter> m_pComputeDispatcher = nullptr;

};


#endif
