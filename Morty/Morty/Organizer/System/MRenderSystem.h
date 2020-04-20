/**
 * @File         MRenderSystem
 * 
 * @Created      2020-04-19 16:05:49
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERSYSTEM_H_
#define _M_MRENDERSYSTEM_H_
#include "MGlobal.h"
#include "MISystem.h"

class MScene;
class MCamera;
class MIMeshInstance;
class MModelInstance;

class MORTY_CLASS MRenderSystem : public MISystem
{
public:
	M_OBJECT(MRenderSystem);
    MRenderSystem();
    virtual ~MRenderSystem();

public:

	void Initialize(MScene* pScene);
	void Release();


    virtual void Tick(const float& fDelta) override;
    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport) override;

protected:


	struct MRenderInfo
	{
		MIRenderer* pRenderer;
		MViewport* pViewport;
	};

	void GenerateShadowMap(MRenderInfo& info);
	void UpdateShaderSharedParams(MRenderInfo& info);
	void DrawMeshInstance(MRenderInfo& info);
	void DrawModelInstance(MRenderInfo& info);
	void DrawSkyBox(MRenderInfo& info);
	void DrawPainter(MRenderInfo& info);
	void DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns);
	void DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pModelIns);
	void DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera);

private:

	MScene* m_pScene;
};

#endif
