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


class MIMesh;
class MScene;
class MEngine;
class MMaterial;
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
	void UnregisterComponent(MComponent* pComponent) override;

	void SceneTick(MScene* pScene, const float& fDelta) override;

	size_t GetInstanceNum() const;
	MBuffer* GetTransformBuffer() { return &m_transformBuffer.buffer; }
	MBuffer* GetInstanceBuffer() { return &m_cullingBuffer.buffer; }


	void OnTransformChanged(MComponent* pComponent);
	void OnMeshChanged(MComponent* pComponent);
	void OnGenerateShadowChanged(MComponent* pComponent);



protected:

	void AddQueueUpdateTransform(MRenderableMeshComponent* pComponent);
	void AddQueueUpdateMesh(MRenderableMeshComponent* pComponent);
	void AddQueueUpdateGenerateShadow(MRenderableMeshComponent* pComponent);


	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeBuffer();
	void ReleaseBuffer();

	void AddComponentToCache(MRenderableMeshComponent* pComponent);
	void RemoveComponentFromCache(MRenderableMeshComponent* pComponent);
	void UpdateBoundsInWorld(MRenderableMeshComponent* pComponent);
	void UpdateComponentMesh(MRenderableMeshComponent* pComponent);
	void UpdateGenerateShadow(MRenderableMeshComponent* pComponent);
	void UpdateInstanceBounds(MShadowMeshInstance* pInstance, Vector3 worldPosition, Vector3 worldScale);

	void UploadCullingBuffer(MRenderableMeshComponent* pComponent);
	void UploadCullingBuffer(MShadowMeshInstance* pInstance);
	void UploadTransformBuffer(MRenderableMeshComponent* pComponent);

private:

	std::set<MRenderableMeshComponent*> m_tWaitUpdateTransformComponent;
	std::set<MRenderableMeshComponent*> m_tWaitUpdateMeshComponent;
	std::set<MRenderableMeshComponent*> m_tWaitUpdateGenerateShadowComponent;
	std::set<MRenderableMeshComponent*> m_tWaitUploadCullingBufferComponent;

	MRenderInstanceCache<MRenderableMeshComponent*, MShadowMeshInstance> m_tShadowInstanceCache;

	MStorageVariant m_transformBuffer;
	MStorageVariant m_cullingBuffer;
};

#endif