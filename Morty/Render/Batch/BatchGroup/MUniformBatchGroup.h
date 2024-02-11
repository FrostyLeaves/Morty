#pragma once

#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Variant/MVariant.h"
#include "MRenderInstanceCache.h"
#include "MInstanceBatchGroup.h"
#include "Shader/MShaderProgram.h"

MORTY_SPACE_BEGIN

struct MShaderConstantParam;
class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderProgram;
class MSceneComponent;
class MRenderMeshComponent;

class MORTY_API MUniformBatchGroup : public MInstanceBatchGroup
{
public:
	struct MTransformVariant
	{
		MVariant worldMatrix;
		MVariant normalMatrix;
		MVariant instanceIndex;
	};

public:

	void Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram) override;
	void Release(MEngine* pEngine) override;
	bool CanAddMeshInstance() const override;
	void AddMeshInstance(const MMeshInstanceRenderProxy& proxy) override;
	void RemoveMeshInstance(MMeshInstanceKey key) override;
	void UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy) override;
	std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const override { return m_pShaderPropertyBlock; }
	MMeshInstanceRenderProxy* FindMeshInstance(MMeshInstanceKey key) override;
	void InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func) override;

private:

	MEngine* m_pEngine = nullptr;
	std::shared_ptr<MShaderProgram> m_pShaderProgram = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pTransformParam = nullptr;
	MRenderInstanceCache<MMeshInstanceKey, MMeshInstanceRenderProxy> m_tInstanceCache;

	std::vector<MTransformVariant> m_vTransformArray;
	size_t m_nCurrentInstanceNum = 0;
	size_t m_nMaxInstanceNum = 0;
};

MORTY_SPACE_END