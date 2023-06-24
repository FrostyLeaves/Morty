#ifndef _M_MATERIAL_BATCH_GROUP_H_
#define _M_MATERIAL_BATCH_GROUP_H_

#include "BatchGroup/MInstanceBatchGroup.h"
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
class MRenderMeshComponent;


class MORTY_API MMaterialBatchGroup
{
public:
	void Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial);
	void Release(MEngine* pEngine);

	static MMeshInstanceRenderProxy CreateProxyFromComponent(MRenderMeshComponent* pComponent);

	void AddMeshInstance(const MMeshInstanceRenderProxy& proxy);
	void UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy);
	void UpdateOrCreateMeshInstance(const MMeshInstanceRenderProxy& proxy);
	void RemoveMeshInstance(MMeshInstanceKey nProxyId);

	bool IsEmpty() const;
	std::shared_ptr<MMaterial> GetMaterial() const { return m_pMaterial; }

	const std::vector<MInstanceBatchGroup*>& GetInstanceBatchGroup() const { return m_vBatchGroup; }


public:

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::map<MMeshInstanceKey, size_t> m_tMeshInstanceTable;
	std::vector<MInstanceBatchGroup*> m_vBatchGroup;

	MEngine* m_pEngine = nullptr;
};


#endif