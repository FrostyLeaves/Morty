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

#include "MMaterialGroup.h"

#include <functional>
#include <vector>

class MNode;
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
class MISystem;
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
	std::vector<MTransparentRenderTarget*>* GetTransparentRenderTarget() { return &m_vTransparentRenderTarget; }

// 	std::vector<MMaterialGroup*>* GetMaterialGroups() { return &m_vMaterialOrderGroups; }
// 	std::vector<MIMeshInstance*>* GetZOrderGroups() { return &m_vZOrderGroups; }
public:

	virtual void Tick(const float& fDelta);
	virtual void Render(MIRenderer* pRenderer, MViewport* pViewport);
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

	void GetSceneRenderMeshAABB(MBoundsAABB& cSceneAABB, MViewport* pViewport);

// 	void RecordMeshInstance(MIMeshInstance* pMeshInstance);
// 	void CancelRecordMeshInstance(MIMeshInstance* pMeshInstance);
//
// 	void InsertMaterialGroup(MIMeshInstance* pMeshInstance);
// 	void RemoveMaterialGroup(MIMeshInstance* pMeshInstance);
// 
// 	void InsertZOrderGroup(MIMeshInstance* pMeshInstance);
// 	void RemoveZOrderGroup(MIMeshInstance* pMeshInstance);

protected:


private:

#define MSCENE_TYPED_VECTOR( TYPE ) \
	public: std::vector<M##TYPE*>* GetAll##TYPE() {return &m_v##TYPE;} \
	private: std::vector<M##TYPE*> m_v##TYPE;

	MNode* m_pRootNode;
	MSkyBox* m_pSkyBox;
	MTransformCoord3D* m_pTransformCoord3D;
	MShadowTextureRenderTarget* m_pShadowDepthMapRenderTarget;
	std::vector<MTransparentRenderTarget*> m_vTransparentRenderTarget;

	MSCENE_TYPED_VECTOR(DirectionalLight);
	MSCENE_TYPED_VECTOR(PointLight);
	MSCENE_TYPED_VECTOR(SpotLight);
	MSCENE_TYPED_VECTOR(InputNode);
	MSCENE_TYPED_VECTOR(ModelInstance);
	MSCENE_TYPED_VECTOR(IModelMeshInstance);

// 	std::vector<MMaterialGroup*> m_vMaterialOrderGroups;
// 	std::vector<MIMeshInstance*> m_vZOrderGroups;

	std::vector<MViewport*> m_vViewports;
	std::vector<MISystem*> m_vSystems;
};

#endif
