#ifndef _M_RENDERABLE_MESH_GROUP_H_
#define _M_RENDERABLE_MESH_GROUP_H_

#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

struct MShaderConstantParam;
class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MRenderableMeshComponent;

struct MORTY_API MRenderableMeshInstance
{
	bool bValid = false;
	MIMesh* pMesh = nullptr;
	MBoundsOBB bounds;
	MBoundsAABB boundsWithTransform;
	MVariant worldMatrixParam;
	MVariant normalMatrixParam;
};

class MORTY_API MRenderableMeshGroup
{
public:

	void Initialize(MEngine* pEngine);
	void Release(MEngine* pEngine);

	void BindMaterial(std::shared_ptr<MMaterial> pMaterial);

	bool canAddMeshInstance() const;

	void AddMeshInstance(MRenderableMeshComponent* pComponent);
	void RemoveMeshInstance(MRenderableMeshComponent* pComponent);
	void UpdateTransform(MRenderableMeshComponent* pComponent);

	const std::vector<MRenderableMeshInstance>& GetMeshInstance() const { return m_vInstances; }
	const std::shared_ptr<MShaderPropertyBlock>& GetMeshProperty() const { return m_pShaderPropertyBlock; }

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pTransformParam = nullptr;

	std::map<MRenderableMeshComponent*, size_t> m_tMeshComponentTable;
	std::vector<MRenderableMeshInstance> m_vInstances;


	bool m_bBatchInstancing = false;
	size_t m_nCurrentInstanceNum = 0;
	size_t m_nMaxInstanceNum = 1;
	MEngine* m_pEngine = nullptr;

	MRepeatIDPool<size_t> m_instancePool;
};

class MORTY_API MRenderableMaterialGroup
{
public:
	void Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial);
	void Release(MEngine* pEngine);

	void AddMeshInstance(MRenderableMeshComponent* pComponent);
	void RemoveMeshInstance(MRenderableMeshComponent* pComponent);
	void UpdateTransform(MRenderableMeshComponent* pComponent);

	bool IsEmpty() const;
	std::shared_ptr<MMaterial> GetMaterial() const { return m_pMaterial; }

	const std::vector<MRenderableMeshGroup*>& GetRenderableMeshGroup() const { return m_vRenderableMeshGroup; }

public:

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::map<MRenderableMeshComponent*, size_t> m_tMeshComponentTable;
	std::vector<MRenderableMeshGroup*> m_vRenderableMeshGroup;

	MEngine* m_pEngine = nullptr;
};


#endif