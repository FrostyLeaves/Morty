/**
 * @File         MFloorRender
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_FLOOR_RENDER_H_
#define _M_FLOOR_RENDER_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

class MScene;
class MSkyBoxComponent;
class MRenderableMaterialGroup;
class MORTY_API MFloorRender : public IRenderable
{
public:

	void SetScene(MScene* pScene);
	void SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);

	void Render(MIRenderCommand* pCommand) override;

private:

	MScene* m_pScene= nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
};


#endif
