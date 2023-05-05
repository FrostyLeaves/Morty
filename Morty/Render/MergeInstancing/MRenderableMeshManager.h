#ifndef _M_RENDERABLE_MESH_MANAGER_H_
#define _M_RENDERABLE_MESH_MANAGER_H_

#include "MRenderableMeshGroup.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderConstantParam;
class MRenderableMeshComponent;


class MORTY_API MRenderableMeshManager : public IManager
{
public:
	MORTY_INTERFACE(MRenderableMeshManager)

	struct MMeshReferenceMap
	{
		MIMesh* pMesh = nullptr;
		std::set<MRenderableMeshComponent*> tComponents;
	};

public:

	virtual void Initialize() override;
	virtual void Release() override;

	virtual void SceneTick(MScene* pScene, const float& fDelta) override;


public:

	void OnTransformChanged(MComponent* pComponent);
	void OnMaterialChanged(MComponent* pComponent);
	void OnMeshChanged(MComponent* pComponent);
	void OnVisibleChanged(MComponent* pComponent);

	void AddQueueUpdateTransform(MRenderableMeshComponent* pComponent);
	void AddQueueUpdateMesh(MRenderableMeshComponent* pComponent);
	void AddQueueUpdateRenderGroup(MRenderableMeshComponent* pComponent);
	void AddQueueUpdateVisible(MRenderableMeshComponent* pComponent);
	void RemoveComponent(MRenderableMeshComponent* pComponent);

	const std::map<std::shared_ptr<MMaterial>, MRenderableMaterialGroup*>& GetRenderableMaterialGroup() const { return m_tRenderableMaterialGroup; };

	std::vector<MRenderableMaterialGroup*> FindGroupFromMaterialType(MEMaterialType eType) const;
	std::vector<MRenderableMaterialGroup*> GetAllMaterialGroup() const;

protected:

	void AddComponentToGroup(MRenderableMeshComponent* pComponent);
	void RemoveComponentFromGroup(MRenderableMeshComponent* pComponent);
	
	void BindMesh(MRenderableMeshComponent* pComponent, MIMesh* pMesh);
	
	void UpdateTransform(MSceneComponent* pComponent);
	void UpdateTransform(MRenderableMeshComponent* pComponent);

	void UpdateVisible(MRenderableMeshComponent* pComponent);

	void Clean();

private:

	std::set<MRenderableMeshComponent*> m_tWaitUpdateTransformComponent;
	std::set<MRenderableMeshComponent*> m_tWaitUpdateMeshComponent;
	std::set<MRenderableMeshComponent*> m_tWaitUpdateRenderGroupComponent;
	std::set<MRenderableMeshComponent*> m_tWaitUpdateVisibleComponent;

	std::map<MRenderableMeshComponent*, MRenderableMaterialGroup*> m_tComponentTable;
	std::map<std::shared_ptr<MMaterial>,MRenderableMaterialGroup*> m_tRenderableMaterialGroup;


	std::map<MIMesh*, MMeshReferenceMap*> m_tMeshReferenceTable;
	std::map<MRenderableMeshComponent*, MMeshReferenceMap*> m_tMeshReferenceComponentTable;
};

#endif