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

struct MMaterialCullingGroup
{
	std::shared_ptr<MMaterial> pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> pMeshTransformProperty = nullptr;
	size_t nClusterBeginIdx = 0;
	size_t nClusterCount = 0;
	size_t nTransformCount = 0;
	const MBuffer* pVertexBuffer = nullptr;
	const MBuffer* pIndexBuffer = nullptr;
};

struct MORTY_API MCascadedShadowRenderData
{
	Vector4 fSplitRange;//far, far + 0.1
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

	/************************** shadow **************************/
	std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> shadowRenderInfo;

	/************************** skybox **************************/
	MEntity* pSkyBoxEntity;

	/************************** mesh **************************/
	// transparent
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>> m_tTransparentGroupMesh;

	//debug
	std::vector<MDebugMeshComponent*> m_vDebugMeshComponent;

public:

};



#endif