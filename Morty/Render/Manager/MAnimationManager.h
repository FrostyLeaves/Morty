#ifndef _M_ANIMATION_MANAGER_H_
#define _M_ANIMATION_MANAGER_H_

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
class MShaderConstantParam;
class MShaderPropertyBlock;
class MRenderMeshComponent;

class MORTY_API MAnimationManager : public IManager
{
public:
	MORTY_CLASS(MAnimationManager)

public:

	void Initialize() override;
	void Release() override;

	std::set<const MType*> RegisterComponentType() const override;

	void RegisterComponent(MComponent* pComponent) override;
	void UnregisterComponent(MComponent* pComponent) override;

	void OnSceneParentChanged(MComponent* pComponent);
	void OnPoseChanged(MComponent* pComponent);
	void RemoveComponent(MModelComponent* pComponent);

	void RenderUpdate(MTaskNode* pNode);

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

#endif