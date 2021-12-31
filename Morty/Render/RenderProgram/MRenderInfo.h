#ifndef M_RENDER_INFO_H_
#define M_RENDER_INFO_H_

#include "MGlobal.h"
#include "Vector.h"
#include "MBounds.h"
#include "MComponent.h"
#include "MShaderParamSet.h"

class MIMesh;
class MMaterial;
class MIRenderCommand;
class MSkeletonInstance;
class MRenderableMeshComponent;
struct MRenderInfo
{
	MRenderInfo();

	uint32_t nFrameIndex;

	// basic
	float fDelta;
	float fGameTime;
	class MViewport* pViewport;


	//camera
	MEntity* pCameraEntity;
	MEntity* pDirectionalLightEntity;


	//bounds
	MBoundsAABB cShadowRenderAABB;
	MBoundsAABB cMeshRenderAABB;


	// frame
	MShaderParamSet* pFrameShaderParamSet;
	MIRenderCommand* pPrimaryRenderCommand;


	//renderable

	//shadow
	class MTexture* pShadowMapTexture;
	Matrix4 m4DirLightInvProj; //valid if pDirectionalLightEntity enable.
	std::map<MSkeletonInstance*, std::vector<MRenderableMeshComponent*>> m_tShadowGroupMesh;

	// transparent
	std::map<MMaterial*, std::vector<MRenderableMeshComponent*>> m_tTransparentGroupMesh;

	// mesh
	std::map<MMaterial*, std::vector<MRenderableMeshComponent*>> m_tMaterialGroupMesh;

	// deferred, gbuffer
	std::map<MMaterial*, std::vector<MRenderableMeshComponent*>> m_tDeferredMaterialGroupMesh;


public:

	void CollectRenderMesh();
	void CollectShadowMesh();
};



#endif