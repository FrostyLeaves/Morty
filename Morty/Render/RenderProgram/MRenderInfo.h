#ifndef M_RENDER_INFO_H_
#define M_RENDER_INFO_H_

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MBounds.h"
#include "Component/MComponent.h"
#include "Material/MShaderParamSet.h"

class MIMesh;
class MTexture;
class MMaterial;
class MIRenderCommand;
class MSkeletonInstance;
class MDebugMeshComponent;
class MRenderableMeshComponent;


struct MCascadedShadowInfo
{
	float fSplitDepth;
	Matrix4 m4DirLightInvProj;
};

struct MRenderInfo
{
	MRenderInfo();

	uint32_t nFrameIndex;

	/************************** basic **************************/
	float fDelta;
	float fGameTime;
	class MViewport* pViewport;

	/************************** render **************************/
	MIRenderCommand* pPrimaryRenderCommand;


	/************************** camera **************************/
	MEntity* pCameraEntity;
	MEntity* pDirectionalLightEntity;


	/************************** skybox **************************/
	MEntity* pSkyBoxEntity;


	/************************** scene **************************/
	MBoundsAABB cCaclSceneRenderAABB;


	/************************** shadow **************************/
	std::array<MCascadedShadowInfo, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> cCascadedShadow;
	std::map<std::shared_ptr<MSkeletonInstance>, std::vector<MRenderableMeshComponent*>> m_tShadowGroupMesh;


	/************************** mesh **************************/
	// transparent
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tTransparentGroupMesh;

	// mesh
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tMaterialGroupMesh;

	// deferred, gbuffer
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tDeferredMaterialGroupMesh;


	//debug
	std::vector<MDebugMeshComponent*> m_vDebugMeshComponent;

public:

};



#endif