#pragma once

#include "Batch/BatchGroup/MInstanceBatchGroup.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"
#include "Model/MAnimationRenderProxy.h"

class IPropertyBlockAdapter;
class MTaskNode;
class MSkeletonInstance;
class MModelComponent;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderPropertyBlock;
class MRenderMeshComponent;
struct MShaderConstantParam;

class MORTY_API MAnimationManager : public IManager
{
public:
	MORTY_CLASS(MAnimationManager)

public:

	void Initialize() override;
	void Release() override;

	std::set<const MType*> RegisterComponentType() const override;

	void UnregisterComponent(MComponent* pComponent) override;

	void OnSceneParentChanged(MComponent* pComponent);
	void OnPoseChanged(MComponent* pComponent);
	void RemoveComponent(MModelComponent* pComponent);

	void RenderUpdate(MTaskNode* pNode);
	MTaskNode* GetUpdateTask() const { return m_pUpdateTask; }

	MAnimationBufferData GetAnimationBuffer() const;
	std::shared_ptr<IPropertyBlockAdapter> CreateAnimationPropertyAdapter();

protected:

	MModelComponent* FindAttachedModelComponent(MSceneComponent* pComponent);
	MSkeletonInstance* GetSkeletonInstance(MModelComponent* pCompnoent);

private:

	std::map<MSkeletonInstanceKey, MPoseRenderProxy> m_tWaitUpdateComponent;
	std::set<MSkeletonInstanceKey> m_tWaitRemoveComponent;

    //Render Thread.
	std::shared_ptr<MAnimationRenderGroup> m_pAnimationRenderGroup = nullptr;
	MTaskNode* m_pUpdateTask = nullptr;
};
