/**
 * @File         MAnimationRenderProxy
 * 
 * @Created      2019-12-09 22:37:59
 *
 * @Author       DoubleYe
**/

#ifndef _M_MANIMATION_RENDER_PROXY_H_
#define _M_MANIMATION_RENDER_PROXY_H_
#include "Basic/MStorageVariant.h"
#include "Batch/BatchGroup/MRenderInstanceCache.h"
#include "Component/MComponent.h"
#include "Render/MRenderGlobal.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "Utility/MGlobal.h"
#include "Variant/MVariant.h"

class MIDevice;
class MSkeletonInstance;
class MEngine;
class MShaderPropertyBlock;

struct MORTY_API MPoseRenderInstance
{
	size_t nMatrixOffset = 0;
	MemoryInfo bonesMemoryInfo;
};

struct MORTY_API MPoseRenderProxy
{
    std::vector<Matrix4> vBoneMatrix;
};

class MORTY_API MAnimationRenderGroup : public IPropertyBlockAdapter
{
public:
	void Initialize(MEngine* pEngine);
	void Release(MEngine* pEngine);

	void AddSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy);
	void RemoveSkeletonRenderInstance(MSkeletonInstanceKey nProxyId);
	void UpdateSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy);
	void UpdateOrCreateMeshInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy);

	std::shared_ptr<MShaderPropertyBlock> GetAnimationProperty() const { return m_pShaderPropertyBlock; }
	std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const override { return GetAnimationProperty(); }

	static MPoseRenderProxy CreatePoseProxy(MSkeletonInstance* pSkeletonInstance);

	MEngine* GetEngine() const { return m_pEngine; }
private:

	MPoseRenderInstance CreatePoseRenderInstance();

	MEngine* m_pEngine = nullptr;
	
	MRenderInstanceCache<MSkeletonInstanceKey, MPoseRenderInstance> m_tPoseRenderInstance;
	MStorageVariant m_bonesStorageBuffer;
	MStorageVariant m_bonesOffsetBuffer;
	MMemoryPool m_bonesStorageMemoryPool = MMemoryPool(0);

	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
};

#endif
