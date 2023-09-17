#pragma once

#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Variant/MVariant.h"
#include "MRenderInstanceCache.h"
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
class MRenderMeshComponent;

class MORTY_API MNoneBatchGroup : public MInstanceBatchGroup
{
public:

	void Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial) override;
	void Release(MEngine* pEngine) override;

	bool CanAddMeshInstance() const override;
	void AddMeshInstance(const MMeshInstanceRenderProxy& proxy) override;
	void RemoveMeshInstance(MMeshInstanceKey key) override;
	void UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy) override;

	std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const override { return m_pShaderPropertyBlock; }
	MMeshInstanceRenderProxy* FindMeshInstance(MMeshInstanceKey key) override { MORTY_UNUSED(key); return &m_instance; }
	void InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func) override;

private:
	MEngine* m_pEngine = nullptr;
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pTransformParam = nullptr;

	MMeshInstanceRenderProxy m_instance;
	bool m_bInstanceValid = false;
	MVariant m_worldMatrix;
	MVariant m_normalMatrix;
	MVariant m_instanceIdx;
};
