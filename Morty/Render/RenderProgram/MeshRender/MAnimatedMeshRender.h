/**
 * @File         MAnimatedMeshRender
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_ANIMATED_MESH_RENDER_H_
#define _M_ANIMATED_MESH_RENDER_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"
#include "RenderProgram/MRenderInfo.h"


class IRenderPassAdapter;
class IPropertyBlockAdapter;

class MORTY_API MAnimatedMeshRender : public MObject
{
	MORTY_CLASS(MAnimatedMeshRender)

public:
	MAnimatedMeshRender();
    ~MAnimatedMeshRender() override;

	void OnCreated() override;
	void OnDelete() override;

	void Initialize(std::shared_ptr<MTexture> pTexture);

	void SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);
	void SetRenderPassAdapter(const std::shared_ptr<IRenderPassAdapter>& pAdapter);
	
	void Render(MRenderInfo& info);


private:

//	void DrawAnimatedMesh(MIRenderCommand* pCommand, const MStaticMeshRenderable& renderable);

	std::shared_ptr<IRenderPassAdapter> m_pRenderPassAdapter = nullptr;
	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
};


#endif
