#ifndef _M_INSTANCE_BATCH_GROUP_H_
#define _M_INSTANCE_BATCH_GROUP_H_

#include "Component/MRenderableMeshComponent.h"
#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Render/MRenderInstanceCache.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

struct MShaderConstantParam;
class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MRenderableMeshComponent;


struct MORTY_API MRenderableMeshInstance
{
	bool bVisible = true;
	MIMesh* pMesh = nullptr;
	MBoundsOBB bounds;
	MBoundsAABB boundsWithTransform;
};

class MORTY_API MInstanceBatchGroup
{
public:
	virtual void Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial) = 0;
	virtual void Release(MEngine* pEngine) = 0;
	virtual bool CanAddMeshInstance() const = 0;
	virtual void AddMeshInstance(MRenderableMeshComponent* pComponent) = 0;
	virtual void RemoveMeshInstance(MRenderableMeshComponent* pComponent) = 0;
	virtual std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const = 0;
	virtual MRenderableMeshInstance* FindMeshInstance(MRenderableMeshComponent* pComponent) = 0;
	virtual void InstanceExecute(std::function<void(const MRenderableMeshInstance&, size_t nIdx)> func) = 0;

	virtual void UpdateTransform(MRenderableMeshComponent* pComponent) = 0;

	void UpdateMesh(MRenderableMeshComponent* pComponent);
	void UpdateVisible(MRenderableMeshComponent* pComponent, bool bVisible);
};

#endif