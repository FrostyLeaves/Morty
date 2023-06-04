#ifndef _M_RENDERABLE_MESH_GROUP_H_
#define _M_RENDERABLE_MESH_GROUP_H_

#include "InstanceBatch/MInstanceBatchGroup.h"
#include "Object/MObject.h"
#include "Utility/MGlobal.h"

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

	static MMeshInstanceRenderProxy CreateProxyFromComponent(MRenderableMeshComponent* pComponent);

	void AddMeshInstance(MMeshInstanceKey key, MMeshInstanceRenderProxy proxy);
	void UpdateMeshInstance(MMeshInstanceKey key, MMeshInstanceRenderProxy proxy);
	void RemoveMeshInstance(MMeshInstanceKey key);

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