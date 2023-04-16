/**
 * @File         MGPUDrivenRender
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

class MScene;
class MRenderableMaterialGroup;
class MORTY_API MGPUDrivenRender : public IRenderable
{
public:

	void SetScene(MScene* pScene);

	void SetDrawIndirect(const std::shared_ptr<IDrawIndirectAdapter>& m_pDrawIndirectAdapter);
	void SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);
	void SetMeshInstancePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);
	void Render(MIRenderCommand* pCommand) override;

private:

	MScene* m_pScene= nullptr;
	std::shared_ptr<IDrawIndirectAdapter> m_pDrawIndirectAdapter = nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pMeshPropertyAdapter = nullptr;
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
