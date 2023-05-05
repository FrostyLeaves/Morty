#ifndef _M_RENDERABLE_MATERIAL_GROUP_H_
#define _M_RENDERABLE_MATERIAL_GROUP_H_

#include "InstanceBatch/MInstanceBatchGroup.h"
#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Render/MRenderInstanceCache.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MShaderPropertyBlock;
struct MShaderConstantParam;
class MRenderableMeshComponent;


class MORTY_API MRenderableMaterialGroup
{
public:
	void Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial);
	void Release(MEngine* pEngine);

	void AddMeshInstance(MRenderableMeshComponent* pComponent);
	void RemoveMeshInstance(MRenderableMeshComponent* pComponent);
	void UpdateTransform(MRenderableMeshComponent* pComponent);
	void UpdateMesh(MRenderableMeshComponent* pComponent);
	void UpdateVisible(MRenderableMeshComponent* pComponent, bool bVisible);

	bool IsEmpty() const;
	std::shared_ptr<MMaterial> GetMaterial() const { return m_pMaterial; }

	const std::vector<MInstanceBatchGroup*>& GetInstanceBatchGroup() const { return m_vRenderableMeshGroup; }

public:

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::map<MRenderableMeshComponent*, size_t> m_tMeshComponentTable;
	std::vector<MInstanceBatchGroup*> m_vRenderableMeshGroup;

	MEngine* m_pEngine = nullptr;
};


#endif