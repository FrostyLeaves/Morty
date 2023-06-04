#ifndef _M_SHADOWMAP_MANAGER_H_
#define _M_SHADOWMAP_MANAGER_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"

#include "Basic/MStorageVariant.h"
#include "Render/MRenderInstanceCache.h"
#include "Shadow/MShadowMapUtil.h"
#include "Component/MRenderableMeshComponent.h"
#include "MergeInstancing/MRenderableMaterialGroup.h"


class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MTaskNode;
class MComponent;
class MComputeDispatcher;
class MShaderConstantParam;
class MShaderPropertyBlock;
class MRenderableMeshComponent;


class MORTY_API MShadowMapManager : public IManager
{
public:
	MORTY_CLASS(MShadowMapManager)

	struct MShadowMeshInstance
	{
		MIMesh* pMesh = nullptr;
		MemoryInfo transformBuffer;
		MemoryInfo cullingBuffer;
		MBoundsSphere boundsSphere;
		MBoundsSphere boundsInWorld;
	};

public:

	void Initialize() override;
	void Release() override;

	std::set<const MType*> RegisterComponentType() const override;
	void RegisterComponent(MComponent* pComponent) override;
	void UnregisterComponent(MComponent* pComponent) override;

	void RenderUpdate(MTaskNode* pNode);
	MTaskNode* GetUpdateTask() const { return m_pUpdateTask; }

	void OnTransformChanged(MComponent* pComponent);
	void OnMeshChanged(MComponent* pComponent);
	void OnGenerateShadowChanged(MComponent* pComponent);

	bool IsGenerateShadowMaterial(MEMaterialType eType) const;

	MRenderableMaterialGroup* GetStaticShadowGroup() { return &m_staticShadowGroup; }


protected:

	void AddToUpdateQueue(MRenderableMeshComponent* pComponent);


	void InitializeMaterial();
	void ReleaseMaterial();

	void RemoveComponentFromCache(MRenderableMeshComponent* pComponent);

private:

	std::map<MRenderableMeshComponent*, MMeshInstanceRenderProxy> m_tWaitAddComponent;
	std::map<MRenderableMeshComponent*, MMeshInstanceRenderProxy> m_tWaitUpdateComponent;
	std::set<MRenderableMeshComponent*> m_tWaitRemoveComponent;
	
	MRenderableMaterialGroup m_staticShadowGroup;
	MTaskNode* m_pUpdateTask = nullptr;
};

#endif