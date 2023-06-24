#ifndef _M_MESH_INSTANCE_MANAGER_H_
#define _M_MESH_INSTANCE_MANAGER_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"
#include "MMaterialBatchGroup.h"

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MTaskNode;
class MMaterial;
class MComponent;
class MShaderConstantParam;
class MRenderMeshComponent;


class MORTY_API MMeshInstanceManager : public IManager
{
public:
	MORTY_INTERFACE(MMeshInstanceManager)

public:

	virtual void Initialize() override;
	virtual void Release() override;

    void RenderUpdate(MTaskNode* pNode);
	MTaskNode* GetUpdateTask() const { return m_pUpdateTask; }
public:

	void OnMaterialChanged(MComponent* pComponent);
	void OnMeshChanged(MComponent* pComponent);
	void OnSceneComponentChanged(MComponent* pComponent);
	void OnRenderMeshChanged(MComponent* pComponent);

	void RemoveComponent(MRenderMeshComponent* pComponent);

	std::vector<MMaterialBatchGroup*> FindGroupFromMaterialType(MEMaterialType eType) const;
	std::vector<MMaterialBatchGroup*> GetAllMaterialGroup() const;


protected:

	bool IsRenderableMeshMaterial(MEMaterialType eType) const;
	void AddComponentToGroup(MRenderMeshComponent* pComponent);
	void RemoveComponentFromGroup(MRenderMeshComponent* pComponent);
	void UpdateMeshInstance(MRenderMeshComponent* pComponent, MMeshInstanceRenderProxy proxy);

	void Clean();

private:

	struct MaterialGroup
	{
		MMaterialBatchGroup materialGroup;
		std::map<MMeshInstanceKey, MMeshInstanceRenderProxy> tWaitUpdateComponent;
		std::set<MMeshInstanceKey> tWaitRemoveComponent;
	};

	std::map<MRenderMeshComponent*, MaterialGroup*> m_tComponentTable;
	std::map<std::shared_ptr<MMaterial>, MaterialGroup*> m_tRenderableMaterialGroup;


	MTaskNode* m_pUpdateTask = nullptr;
};

#endif