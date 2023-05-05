#ifndef _M_NONE_BATCH_GROUP_H_
#define _M_NONE_BATCH_GROUP_H_

#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Render/MRenderInstanceCache.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"
#include "MInstanceBatchGroup.h"

struct MShaderConstantParam;
class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MRenderableMeshComponent;

class MORTY_API MNoneBatchGroup : public MInstanceBatchGroup
{
public:

	void Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial) override;
	void Release(MEngine* pEngine) override;

	bool CanAddMeshInstance() const override;
	void AddMeshInstance(MRenderableMeshComponent* pComponent) override;
	void RemoveMeshInstance(MRenderableMeshComponent* pComponent) override;
	void UpdateTransform(MRenderableMeshComponent* pComponent) override;

	std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const override { return m_pShaderPropertyBlock; }
	MRenderableMeshInstance* FindMeshInstance(MRenderableMeshComponent* pComponent) override { return &m_instance; }
	void InstanceExecute(std::function<void(const MRenderableMeshInstance&, size_t nIdx)> func) override;

private:
	MEngine* m_pEngine = nullptr;
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pTransformParam = nullptr;

	MRenderableMeshInstance m_instance;
	bool m_bInstanceValid = false;
	MVariant m_worldMatrix;
	MVariant m_normalMatrix;
};

#endif