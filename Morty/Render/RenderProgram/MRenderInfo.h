#pragma once

#include "Basic/MCameraFrustum.h"
#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MBounds.h"
#include "Component/MComponent.h"
#include "Shader/MShaderPropertyBlock.h"

MORTY_SPACE_BEGIN

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

struct MORTY_API MCascadedSplitData
{
	//range: 0.0 - 1.0f
	float fCascadeSplit = 0.0f;
	float fTransitionRange = 0.0f;
	float fNearZ = 0.0f;
	float fFarZ = 0.0f;
	float fOverFarZ = 0.0f;

	//MCameraFrustum cCameraFrustum;
};

struct MORTY_API MCascadedShadowRenderData
{
	Matrix4 m4DirLightInvProj;
	Vector4 fSplitRange;//far, far + 0.1
};

struct MORTY_API MVoxelClipmap
{
	Vector3 f3VoxelOrigin = {};
	float fVoxelSize = 0.0f;
};

struct MORTY_API MVoxelMapSetting
{
	MVoxelClipmap vClipmap[MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM];
	uint32_t nResolution = 1;
	uint32_t nViewportSize = 1;
	uint32_t nClipmapIdx = 0;
};

struct MORTY_API MDirectionLightData
{
	Vector3 f3LightDirection;
	Vector3 f3LightIntensity;
	float fLightSize;
};

struct MORTY_API MPointLightData
{
	Vector3 f3LightPosition;
	Vector3 f3LightIntensity;

	float fConstant;
	float fLinear;
	float fQuadratic;
};

struct MRenderInfo
{
	//TODO remove scene pointer.
	const MScene* pScene = nullptr;

	/************************** render **************************/
	MIRenderCommand* pPrimaryRenderCommand = nullptr;

	/************************** basic **************************/
	uint32_t nFrameIndex = 0;
	float fDelta = 0.0f;
	float fGameTime = 0.0f;

	Vector2i f2ViewportLeftTop;
	Vector2i f2ViewportSize;

	/************************** camera **************************/
	Vector2 f2CameraNearFar;
	Matrix4 m4ProjectionMatrix;
	Matrix4 m4CameraTransform;
	Matrix4 m4CameraInverseProjection;
	MCameraFrustum cameraFrustum;

	/************************** environment **************************/
	std::shared_ptr<MTexture> pEnvDiffuseTexture = nullptr;
	std::shared_ptr<MTexture> pEnvSpecularTexture = nullptr;

	/************************** light **************************/
	MDirectionLightData directionLight;
	std::vector<MPointLightData> vPointLight;

	/************************** shadow **************************/
	std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> shadowRenderInfo;

public:

	static MRenderInfo CreateFromViewport(MViewport* pViewport);

	static void FillVoxelMapSetting(const MVoxelMapSetting& setting, MVariantStruct& output);
};

MORTY_SPACE_END