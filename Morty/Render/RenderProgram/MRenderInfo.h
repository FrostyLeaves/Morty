#pragma once

#include "Basic/MCameraFrustum.h"
#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MBounds.h"
#include "Component/MComponent.h"
#include "Material/MShaderPropertyBlock.h"

class MIMesh;
class MTexture;
class MMaterial;
class MIRenderCommand;
class MSkeletonInstance;
class MDebugMeshComponent;
class MRenderMeshComponent;

struct MMaterialCullingGroup
{
	std::shared_ptr<MMaterial> pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> pMeshTransformProperty = nullptr;
	size_t nIndirectBeginIdx = 0;
	size_t nIndirectCount = 0;
};

struct MORTY_API MCascadedShadowSceneData
{
	//range: 0.0 - 1.0f
	float fCascadeSplit = 0.0f;
	float fTransitionRange = 0.0f;
	float fNearZ = 0.0f;
	float fFarZ = 0.0f;
	float fOverFarZ = 0.0f;

	MCameraFrustum cCameraFrustum;
};

struct MORTY_API MCascadedShadowRenderData
{
	Matrix4 m4DirLightInvProj;
	Vector4 fSplitRange;//far, far + 0.1
	MBoundsSphere boundsSphere;
};

struct MORTY_API MVoxelMapSetting
{
	Vector3 f3VoxelOrigin = {};
	float fResolution = 0.0f;
	float fVoxelStep = 0.0f;

	const MBuffer* pVoxelTableBuffer = nullptr;
};

struct MRenderInfo
{
	uint32_t nFrameIndex = 0;

	/************************** basic **************************/
	float fDelta = 0.0f;
	float fGameTime = 0.0f;
	class MViewport* pViewport = nullptr;

	/************************** render **************************/
	MIRenderCommand* pPrimaryRenderCommand = nullptr;

	/************************** camera **************************/
	MEntity* pCameraEntity = nullptr;
	MEntity* pDirectionalLightEntity = nullptr;
	Matrix4 m4CameraInverseProjection;
	MCameraFrustum cameraFrustum;

	/************************** shadow **************************/
	std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> shadowRenderInfo;

	/************************** skybox **************************/
	MEntity* pSkyBoxEntity = nullptr;

	/************************** mesh **************************/
	// transparent
	std::map<std::shared_ptr<MMaterial>, std::vector<MRenderMeshComponent*>> m_tTransparentGroupMesh;

	//debug
	std::vector<MDebugMeshComponent*> m_vDebugMeshComponent;

	//voxelizer
	MVoxelMapSetting voxelSetting;

public:

};
