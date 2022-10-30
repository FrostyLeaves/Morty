#ifndef _M_BATCH_RENDER_SUB_SYSTEM_H_
#define _M_BATCH_RENDER_SUB_SYSTEM_H_

#include "Utility/MGlobal.h"
#include "Scene/MSubSystem.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MRenderableMeshComponent;
class MORTY_API MBatchRenderSubSystem : public MISubSystem
{
public:
	MORTY_INTERFACE(MBatchRenderSubSystem)

public:

	struct MSharedMeshComponentGroup
	{
		MIMesh* pMesh;
		std::set<MRenderableMeshComponent*> m_tComponents;
	};

	struct MBatchInstanceData
	{
		std::weak_ptr<MMaterial> pMaterial;
		std::map<MIMesh*, MSharedMeshComponentGroup*> m_tMeshToGroup;
		std::map<MRenderableMeshComponent*, MSharedMeshComponentGroup*> m_tComponentToGroup;
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

	std::map<std::weak_ptr<MMaterial>, MBatchInstanceData*> m_tMaterialToBatchInstanceTable;
	std::map<MRenderableMeshComponent*, MBatchInstanceData*> m_tComponentToBatchInstanceTable;

};

#endif