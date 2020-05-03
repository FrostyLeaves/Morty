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
#include "MBounds.h"
#include "MMaterialGroup.h"
#include "MShadowTextureRenderTarget.h"

#include <vector>

class MScene;
class MCamera;
class MIMeshInstance;
class MModelInstance;
class MSkeletonInstance;
class MDirectionalLight;
class MIModelMeshInstance;
class MORTY_CLASS MRenderSystem : public MISystem
{
public:

	struct MRenderInfo
	{
		MIRenderer* pRenderer;
		MViewport* pViewport;
		MCamera* pCamera;
		MScene* pScene;

		MDirectionalLight* pDirectionalLight;
		Matrix4 m4DirLightInvProj;

		MBoundsAABB cShadowRenderAABB;
		MBoundsAABB cMeshRenderAABB;

		std::vector<MMaterialGroup> vMaterialRenderGroup;
		std::vector<MIMeshInstance*> vTransparentRenderGroup;
		std::vector<MShadowRenderGroup> vShadowGroup;
	};

public:
	M_OBJECT(MRenderSystem);
    MRenderSystem();
    virtual ~MRenderSystem();

public:

	void Initialize();
	void Release();


    virtual void Tick(const float& fDelta) override;
    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene) override;

protected:

	void GenerateRenderGroup(MRenderInfo& info);
	void GenerateShadowMap(MRenderInfo& info);
	void UpdateShaderSharedParams(MRenderInfo& info);
	void DrawMeshInstance(MRenderInfo& info);


	void DrawModelInstance(MRenderInfo& info);
	void DrawSkyBox(MRenderInfo& info);
	void DrawPainter(MRenderInfo& info);
	void DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns);
	void DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pModelIns);
	void DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera);


protected:

	void DrawMeshInstance(MIRenderer* &pRenderer, MIMeshInstance* &pMeshInstance, MShaderParam* &pMeshMatrixParam, MShaderParam* &pAnimationParam);

	void RecordMeshInstance(MRenderInfo& info, MIMeshInstance* pMeshInstance);

};

#endif
