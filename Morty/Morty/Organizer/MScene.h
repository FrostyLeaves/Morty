/**
 * @File         MScene
 * 
 * @Created      2019-09-19 00:32:56
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSCENE_H_
#define _M_MSCENE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Matrix.h"
#include "MEngine.h"

#include "MMaterialGroup.h"

#include <functional>
#include <vector>

class MNode;
class MResource;
class MSkyBox;
class MIRenderer;
class MInputEvent;
class MIRenderView;
class MViewport;
class MMaterial;
class MTransformCoord3D;
class MShadowTextureRenderTarget;
class MBoundsAABB;
class MIRenderTarget;

class MComponent;
class MSpotLightComponent;
class MPointLightComponent;
class MDirectionalLightComponent;
class MORTY_API MComponentGroup
{
public:

	void AddComponent(MComponent* pComponent);
	void RemoveComponent(MComponent* pComponent);

public:

	std::vector<MComponent*> m_vComponent;
};

class MORTY_API MScene : public MObject
{
public:
	M_OBJECT(MScene);
    MScene();
    virtual ~MScene();

public:

	virtual void SetRootNode(MNode* pNode);
	MNode* GetRootNode() { return m_pRootNode; }

public:

	MDirectionalLightComponent* FindActiveDirectionLight();
	void FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLightComponent*>& vPointLights);
	void FindActiveSpotLights(const Vector3& v3WorldPosition, std::vector<MSpotLightComponent*>& vPointLights);

	MSkyBox* GetSkyBox() { return m_pSkyBox; }
	MTransformCoord3D* GetTransformCoord() { return m_pTransformCoord3D; }

	std::vector<MMaterialGroup*>& GetMaterialGroup() { return m_vMaterialGroups; }

	template <class T>
	MComponentGroup* FindComponents();

public:

	//Dont call

	void AddComponent(MComponent* pComponent);
	void RemoveComponent(MComponent* pComponent);

public:

	virtual void Tick(const float& fDelta);
	virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MIRenderTarget* pRenderTarget);
	virtual void Input(MInputEvent* pEvent, MViewport* pViewport);

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	//节点接入场景时进行的操作
	void OnNodeEnter(MNode* pNode);
	//节点退出场景时进行的操作
	void OnNodeExit(MNode* pNode);

	void AddAttachedViewport(MViewport* pViewport);
	void RemoveAttachedViewport(MViewport* pViewport);
	std::vector<MViewport*> GetViewports() { return m_vViewports; }

	void InsertMaterialGroup(MRenderableMeshComponent* pNode);
	void RemoveMaterialGroup(MRenderableMeshComponent* pNode);

private:

	MNode* m_pRootNode;
	MSkyBox* m_pSkyBox;
	MTransformCoord3D* m_pTransformCoord3D;


	std::map<const void*, MComponentGroup*> m_tComponents;

private:
	std::vector<MMaterialGroup*> m_vMaterialGroups;

	std::vector<MViewport*> m_vViewports;
};

template <class T>
MComponentGroup* MScene::FindComponents()
{
	auto findResult = m_tComponents.find(T::GetClassTypeIdentifier());
	if (findResult == m_tComponents.end())
		return nullptr;

	return findResult->second;
}

#endif
