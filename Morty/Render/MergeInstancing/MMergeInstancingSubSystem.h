#ifndef _M_MERGE_INSTANCING_SUB_SYSTEM_H_
#define _M_MERGE_INSTANCING_SUB_SYSTEM_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MSubSystem.h"
#include "MMergeInstancingMesh.h"


class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderConstantParam;
class MRenderableMeshComponent;


class MBatchMeshBuffer : public MBuffer
{
public:
	MBatchMeshBuffer();

	uint32_t AllowMeshCluster();
	void FreeMeshCluster(const uint32_t& idx);

	MByte* GetMeshCluster(const uint32_t& idx);

};

struct MORTY_API MMaterialBatchGroup
{
	std::shared_ptr<MMaterial> pMaterial = nullptr;
	std::set<MRenderableMeshComponent*> vMeshComponent = {};
};

class MORTY_API MMergeInstancingSubSystem : public MISubSystem
{
public:
	MORTY_INTERFACE(MMergeInstancingSubSystem)

public:

	MMergeInstancingSubSystem();
	virtual ~MMergeInstancingSubSystem();

	virtual void Initialize() override;
	virtual void Release() override;

	virtual void SceneTick(MScene* pScene, const float& fDelta) override;


public:

	void RegisterMaterial(MRenderableMeshComponent* pComponent);
	void UnregisterMaterial(MRenderableMeshComponent* pComponent);

	void RegisterMesh(MRenderableMeshComponent* pComponent);
	void UnregisterMesh(MRenderableMeshComponent* pComponent);

	MMaterialBatchGroup* GetMaterialBatchGroup(std::shared_ptr<MMaterial> pMaterial);
	const std::map<std::shared_ptr<MMaterial>, MMaterialBatchGroup*>& GetMaterialToBatchInstanceTable() { return m_tMaterialToBatchInstanceTable; }

	const std::vector<MMergeInstancingMesh::MClusterData>& GetMeshClusterGroup(MIMesh* pMesh);

	const MMergeInstancingMesh* GetMergeInstancingMesh() const { return m_pMergeMesh; }
public:

	void OnBatchMeshChanged(MComponent* pSender);

private:

	MMergeInstancingMesh* m_pMergeMesh;
	std::map<MRenderableMeshComponent*, MIMesh*> m_tComponentMeshTable;
	std::map<MRenderableMeshComponent*, std::shared_ptr<MMaterial>> m_tComponentMaterialTable;
	std::map<MIMesh*, int> m_tMeshReferenceCount;

	std::map<std::shared_ptr<MMaterial>, MMaterialBatchGroup*> m_tMaterialToBatchInstanceTable;
	
};

#endif