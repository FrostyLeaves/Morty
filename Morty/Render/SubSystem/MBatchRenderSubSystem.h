#ifndef _M_BATCH_RENDER_SUB_SYSTEM_H_
#define _M_BATCH_RENDER_SUB_SYSTEM_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MSubSystem.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MRenderableMeshComponent;


class MBatchMeshBuffer : public MBuffer
{
public:
	struct MMeshCluster
	{
		size_t unBeginOffset;
	};


public:
	MBatchMeshBuffer();

	uint32_t AllowMeshCluster();
	void FreeMeshCluster(const uint32_t& idx);

	MByte* GetMeshCluster(const uint32_t& idx);

private:
	
	std::map<uint32_t, MMeshCluster> m_tClusterTable;

	static constexpr size_t ClusterSize = 64u;
};


class MORTY_API MBatchRenderSubSystem : public MISubSystem
{
public:
	MORTY_INTERFACE(MBatchRenderSubSystem)

public:

	struct MSharedMeshData
	{
		MIMesh* pMesh;
		std::set<MRenderableMeshComponent*> m_tComponents;
		std::vector<size_t> m_vClusters;
	};

	struct MMaterialBatchGroup
	{
		std::weak_ptr<MMaterial> pMaterial;
		std::map<MIMesh*, MSharedMeshData*> m_tSharedMesh;
		std::map<MRenderableMeshComponent*, MSharedMeshData*> m_tComponentToSharedMesh;

		MBatchMeshBuffer m_meshBuffer;
	};

public:

	MBatchRenderSubSystem();
	virtual ~MBatchRenderSubSystem();

	virtual void Initialize() override;
	virtual void Release() override;

	virtual void SceneTick(MScene* pScene, const float& fDelta) override;


public:

	void RegisterComponent(MRenderableMeshComponent* pComponent);
	void UnregisterComponent(MRenderableMeshComponent* pComponent);

public:

	void OnBatchMeshChanged(MComponent* pSender);

private:

	std::map<std::weak_ptr<MMaterial>, MMaterialBatchGroup*> m_tMaterialToBatchInstanceTable;
	std::map<MRenderableMeshComponent*, MMaterialBatchGroup*> m_tComponentToBatchInstanceTable;

};

#endif