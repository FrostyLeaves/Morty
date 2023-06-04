#ifndef _M_RENDERABLE_MESH_MANAGER_H_
#define _M_RENDERABLE_MESH_MANAGER_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"
#include "MRenderableMaterialGroup.h"

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MTaskNode;
class MMaterial;
class MComponent;
class MShaderConstantParam;
class MRenderableMeshComponent;


class MORTY_API MRenderableMeshManager : public IManager
{
public:
	MORTY_INTERFACE(MRenderableMeshManager)

public:

	virtual void Initialize() override;
	virtual void Release() override;

    void RenderUpdate(MTaskNode* pNode);
	MTaskNode* GetUpdateTask() const { return m_pUpdateTask; }
public:

	void OnTransformChanged(MComponent* pComponent);
	void OnMaterialChanged(MComponent* pComponent);
	void OnMeshChanged(MComponent* pComponent);
	void OnVisibleChanged(MComponent* pComponent);

	void RemoveComponent(MRenderableMeshComponent* pComponent);

	std::vector<MRenderableMaterialGroup*> FindGroupFromMaterialType(MEMaterialType eType) const;
	std::vector<MRenderableMaterialGroup*> GetAllMaterialGroup() const;


protected:

	bool IsRenderableMeshMaterial(MEMaterialType eType) const;
	void AddComponentToGroup(MRenderableMeshComponent* pComponent);
	void RemoveComponentFromGroup(MRenderableMeshComponent* pComponent);
	void UpdateMeshInstance(MRenderableMeshComponent* pComponent, MMeshInstanceRenderProxy proxy);

	void Clean();

private:

	struct MaterialGroup
	{
		MRenderableMaterialGroup materialGroup;
		std::map<MRenderableMeshComponent*, MMeshInstanceRenderProxy> tWaitAddComponent;
		std::map<MRenderableMeshComponent*, MMeshInstanceRenderProxy> tWaitUpdateComponent;
		std::set<MRenderableMeshComponent*> tWaitRemoveComponent;
	};

	std::map<MRenderableMeshComponent*, MaterialGroup*> m_tComponentTable;
	std::map<std::shared_ptr<MMaterial>, MaterialGroup*> m_tRenderableMaterialGroup;


	MTaskNode* m_pUpdateTask = nullptr;
};

#endif