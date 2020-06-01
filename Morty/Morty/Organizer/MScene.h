/**
 * @File         MScene
 * 
 * @Created      2019-09-19 00:32:56
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSCENE_H_
#define _M_MSCENE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Matrix.h"
#include "MEngine.h"
#include "MISystem.h"

#include "MMaterialGroup.h"

#include <functional>
#include <vector>

class MNode;
class MResource;
class MModelInstance;
class MCamera;
class MSkyBox;
class MDirectionalLight;
class MPointLight;
class MSpotLight;
class MIRenderer;
class MInputEvent;
class MIRenderView;
class MViewport;
class MMaterial;
class MIMeshInstance;
class MIModelMeshInstance;
class MTransformCoord3D;
class MInputNode;
class MShadowTextureRenderTarget;
class MTransparentRenderTarget;
class MBoundsAABB;
class MIRenderTarget;
class MORTY_CLASS MScene : public MObject
{
public:
	M_OBJECT(MScene);
    MScene();
    virtual ~MScene();

public:

	virtual void SetRootNode(MNode* pNode);
	MNode* GetRootNode() { return m_pRootNode; }

public:

	MDirectionalLight* FindActiveDirectionLight();
	void FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLight*>& vPointLights);
	void FindActiveSpotLights(const Vector3& v3WorldPosition, std::vector<MSpotLight*>& vPointLights);

	MSkyBox* GetSkyBox() { return m_pSkyBox; }
	MTransformCoord3D* GetTransformCoord() { return m_pTransformCoord3D; }
	MShadowTextureRenderTarget* GetShadowRenderTarget() { return m_pShadowDepthMapRenderTarget; }
	MTransparentRenderTarget* GetTransparentRenderTarget() { return m_pTransparentRenderTarget; }

	std::vector<MMaterialGroup*>& GetMaterialGroup() { return m_vMaterialGroups; }

public:

	template<typename SYSTEM>
	void RegisterSystem()
	{
		if (MTypedClass::IsType<SYSTEM, MISystem>())
		{
			SYSTEM* pSystem = (SYSTEM*)(m_pEngine->GetObjectManager()->CreateObject<SYSTEM>());
			RegisterSystem(pSystem);
		}
	}

public:

	virtual void Tick(const float& fDelta);
	virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MIRenderTarget* pRenderTarget);
	virtual void Input(MInputEvent* pEvent, MViewport* pViewport);

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void CleanAllNodes();

	//节点接入场景时进行的操作
	void OnNodeEnter(MNode* pNode);
	//节点退出场景时进行的操作
	void OnNodeExit(MNode* pNode);

	void AddAttachedViewport(MViewport* pViewport);
	void RemoveAttachedViewport(MViewport* pViewport);
	std::vector<MViewport*> GetViewports() { return m_vViewports; }

	void InsertMaterialGroup(MIMeshInstance* pMeshInstance);
	void RemoveMaterialGroup(MIMeshInstance* pMeshInstance);

public:

	void Load(MResource* pResource);

protected:

	void RegisterSystem(MISystem* pSystem);

private:

#define MSCENE_TYPED_VECTOR( TYPE ) \
	public: std::vector<M##TYPE*>* GetAll##TYPE() {return &m_v##TYPE;} \
	private: std::vector<M##TYPE*> m_v##TYPE;

	MNode* m_pRootNode;
	MSkyBox* m_pSkyBox;
	MTransformCoord3D* m_pTransformCoord3D;
	MShadowTextureRenderTarget* m_pShadowDepthMapRenderTarget;
	MTransparentRenderTarget* m_pTransparentRenderTarget;

	MSCENE_TYPED_VECTOR(DirectionalLight);
	MSCENE_TYPED_VECTOR(PointLight);
	MSCENE_TYPED_VECTOR(SpotLight);
	MSCENE_TYPED_VECTOR(InputNode);
	MSCENE_TYPED_VECTOR(ModelInstance);

private:
	std::vector<MMaterialGroup*> m_vMaterialGroups;

	std::vector<MViewport*> m_vViewports;
	std::vector<MISystem*> m_vSystems;
};

#endif
