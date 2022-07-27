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
class MDebugMeshComponent;
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

	// skybox
	MEntity* pSkyBoxEntity;

	//bounds
	MBoundsAABB cGenerateShadowRenderAABB;
	MBoundsAABB cCaclShadowRenderAABB;


	// frame
	MShaderParamSet* pFrameShaderParamSet;
	MIRenderCommand* pPrimaryRenderCommand;


	//renderable

	//shadow
	class MTexture* pShadowMapTexture;
	Matrix4 m4DirLightInvProj; //valid if pDirectionalLightEntity enable.
	std::map<std::shared_ptr<MSkeletonInstance>, std::vector<MRenderableMeshComponent*>> m_tShadowGroupMesh;

	// transparent
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tTransparentGroupMesh;

	// mesh
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tMaterialGroupMesh;

	// deferred, gbuffer
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tDeferredMaterialGroupMesh;

	//debug
	std::vector<MDebugMeshComponent*> m_vDebugMeshComponent;

public:

	void CollectRenderMesh();
	void CollectShadowMesh();
};



#endif