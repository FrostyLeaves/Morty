#include "MScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"

#include "MISystem.h"
#include "MRenderSystem.h"

#include "Light/MDirectionalLight.h"
#include "Light/MPointLight.h"
#include "Light/MSpotLight.h"

#include "Model/MStaticMeshInstance.h"
#include "Model/MModelMeshStruct.h"
#include "Model/MIModelMeshInstance.h"
#include "Material/MMaterialResource.h"

#include "MVertex.h"
#include "MMaterial.h"
#include "MIRenderer.h"
#include "MIRenderView.h"
#include "MViewport.h"
#include "MTransformCoord.h"
#include "Model/MModelInstance.h"
#include "MBounds.h"
#include "Model/MModelResource.h"
#include "MPainter.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"
#include "MInputNode.h"
#include "MTexture.h"
#include "MTextureRenderTarget.h"

#include "MSkeleton.h"
#include "Texture/MTextureResource.h"

#if MORTY_RENDER_DATA_STATISTICS
#include "MRenderStatistics.h"
#endif

#include "MShadowTextureRenderTarget.h"

#include <algorithm>

MTypeIdentifierImplement(MScene, MObject)


#define MSCENE_ON_NODE_ENTER( TYPE ) \
	else if(M##TYPE* pTypedNode = pNode->DynamicCast<M##TYPE>()) \
		m_v##TYPE.push_back(pTypedNode);

#define MSCENE_ON_NODE_EXIT( TYPE ) \
	else if(M##TYPE* pTypedNode = pNode->DynamicCast<M##TYPE>()) {\
		std::vector<M##TYPE*>::iterator iter = std::find(m_v##TYPE.begin(), m_v##TYPE.end(), pTypedNode); \
		if (m_v##TYPE.end() != iter) \
			m_v##TYPE.erase(iter); \
	}

MScene::MScene()
	: MObject()
	, m_pRootNode(nullptr)
	, m_pSkyBox(nullptr)
	, m_pTransformCoord3D(nullptr)
	, m_pShadowDepthMapRenderTarget(nullptr)
	, m_vViewports()
{
	
}

void MScene::OnCreated()
{
	MObject::OnCreated();

	m_pSkyBox = m_pEngine->GetObjectManager()->CreateObject<MSkyBox>();
	m_pTransformCoord3D = m_pEngine->GetObjectManager()->CreateObject<MTransformCoord3D>();

	m_pShadowDepthMapRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MShadowTextureRenderTarget>();

	MRenderSystem* pRenderSystem = m_pEngine->GetObjectManager()->CreateObject<MRenderSystem>();
	m_vSystems.push_back(pRenderSystem);
}

void MScene::GetSceneRenderMeshAABB(MBoundsAABB& cSceneAABB, MViewport* pViewport)
{
	Vector3 v3ShadowMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3ShadowMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (MIModelMeshInstance* pMeshIns : m_vIModelMeshInstance)
	{
		if (pMeshIns->GetVisibleRecursively())
		{
			const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
			if (pViewport->GetCameraFrustum()->ContainTest(*pBounds) != MCameraFrustum::EOUTSIDE)
			{
				pBounds->UnionMinMax(v3ShadowMin, v3ShadowMax);
			}
		}
	}

	cSceneAABB.SetMinMax(v3ShadowMin, v3ShadowMax);
}

void MScene::AddAttachedViewport(MViewport* pViewport)
{
	for (MViewport* pv : m_vViewports)
		if (pv == pViewport)
			return;

	m_vViewports.push_back(pViewport);


}

void MScene::RemoveAttachedViewport(MViewport* pViewport)
{
	std::vector<MViewport*>::iterator iter = std::find(m_vViewports.begin(), m_vViewports.end(), pViewport);
	if (iter != m_vViewports.end())
	{
		m_vViewports.erase(iter);
	}
}

MDirectionalLight* MScene::FindActiveDirectionLight()
{
	if (m_vDirectionalLight.empty())
		return nullptr;

	return m_vDirectionalLight.back();
}

void MScene::FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLight*>& vPointLights)
{
	static auto compareFunc = [v3WorldPosition](MPointLight* a, MPointLight* b) {
		return (a->GetWorldPosition() - v3WorldPosition).Length() < (b->GetWorldPosition() - v3WorldPosition).Length();
	};

	if (m_vPointLight.size() <= vPointLights.size())
	{
		std::copy(m_vPointLight.begin(), m_vPointLight.end(), vPointLights.begin());
	}
	else if (vPointLights.size() * 2 < m_vPointLight.size())
	{
		std::fill(vPointLights.begin(), vPointLights.end(), nullptr);
		for (MPointLight* pLight : m_vPointLight)
		{
			for (std::vector<MPointLight*>::iterator iter = vPointLights.begin(); iter != vPointLights.end(); ++iter)
			{
				if (*iter == nullptr)
				{
					(*iter) = pLight;
					break;
				}
				else if (compareFunc(pLight, *iter))
				{
					for (std::vector<MPointLight*>::iterator nextIter = vPointLights.end() - 1; nextIter != iter; --nextIter)
						(*nextIter) = *(nextIter - 1);
					(*iter) = pLight;
					break;
				}
			}
		}
	}
	else
	{
		std::sort(m_vPointLight.begin(), m_vPointLight.end(), compareFunc);
		std::copy(m_vPointLight.begin(), m_vPointLight.begin() + vPointLights.size(), vPointLights.begin());
	}
}

void MScene::FindActiveSpotLights(const Vector3& v3WorldPosition, std::vector<MSpotLight*>& vSpotLights)
{
	//TODO 获取所有方向和摄像机一致的聚光灯

	for (unsigned int i = 0; i < vSpotLights.size() && i < m_vSpotLight.size(); ++i)
	{
		vSpotLights[i] = m_vSpotLight[i];
	}
}

void MScene::OnNodeEnter(MNode* pNode)
{
	if (MCamera* pCamera = pNode->DynamicCast<MCamera>())
	{
		for (MViewport* pViewport : m_vViewports)
		{
			if (pViewport->IsUseDefaultCamera())
				pViewport->SetCamera(pCamera);
		}
	}

MSCENE_ON_NODE_ENTER(IModelMeshInstance)
MSCENE_ON_NODE_ENTER(ModelInstance)
MSCENE_ON_NODE_ENTER(DirectionalLight)
MSCENE_ON_NODE_ENTER(PointLight)
MSCENE_ON_NODE_ENTER(SpotLight)
MSCENE_ON_NODE_ENTER(InputNode)
}

void MScene::OnNodeExit(MNode* pNode)
{
	if (MCamera* pCamera = pNode->DynamicCast<MCamera>())
	{
		for (MViewport* pViewport : m_vViewports)
		{
			if (pViewport->GetCamera() == pCamera)
				pViewport->SetCamera(nullptr);
		}
	}

MSCENE_ON_NODE_EXIT(IModelMeshInstance)
MSCENE_ON_NODE_EXIT(ModelInstance)
MSCENE_ON_NODE_EXIT(DirectionalLight)
MSCENE_ON_NODE_EXIT(PointLight)
MSCENE_ON_NODE_EXIT(SpotLight)
MSCENE_ON_NODE_EXIT(InputNode)
}

void MScene::Render(MIRenderer* pRenderer, MViewport* pViewport)
{
#if MORTY_RENDER_DATA_STATISTICS
	MRenderStatistics::GetInstance()->unTriangleCount = 0;
#endif

	for (MISystem* pSystem : m_vSystems)
	{
		pSystem->Render(pRenderer, pViewport, this);
	}
}

void MScene::Tick(const float& fDelta)
{
	if (m_pRootNode)
	{
		m_pRootNode->Tick(fDelta);
	}
}

void MScene::Input(MInputEvent* pEvent, MViewport* pViewport)
{
	m_pTransformCoord3D->Input(pEvent, pViewport);

	for (MInputNode* pNode : m_vInputNode)
	{
		if (pNode->Input(pEvent, pViewport))
			break;
	}
}

MScene::~MScene()
{
	if (m_pShadowDepthMapRenderTarget)
	{
		m_pEngine->GetObjectManager()->RemoveObject(m_pShadowDepthMapRenderTarget->GetObjectID());
		m_pShadowDepthMapRenderTarget = nullptr;
	}
}

void MScene::SetRootNode(MNode* pRootNode)
{
	m_pRootNode = pRootNode;
	if (pRootNode)
	{
		pRootNode->SetAttachedScene(this);
	}
}
// 
// void MScene::RecordMeshInstance(MIModelMeshInstance* pMeshInstance)
// {
// 	if (!pMeshInstance->GetMaterial())
// 		return;
// 
// 	MMaterial* pMaterial = pMeshInstance->GetMaterial();
// 	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vRenderMeshGroup.begin(), m_vRenderMeshGroup.end(), pMaterial, [](MaterialMeshInsGroup* a, MMaterial* b) {return a->pMat < b; });
// 	MaterialMeshInsGroup* pGroup = nullptr;
// 	if (iter == m_vRenderMeshGroup.end())
// 	{
// 		pGroup = new MaterialMeshInsGroup();
// 		pGroup->pMat = pMeshInstance->GetMaterial();
// 		m_vRenderMeshGroup.push_back(pGroup);
// 	}
// 	else if ((*iter)->pMat != pMaterial)
// 	{
// 			pGroup = new MaterialMeshInsGroup();
// 			pGroup->pMat = pMeshInstance->GetMaterial();
// 			m_vRenderMeshGroup.insert(iter, pGroup);
// 	}
// 	else
// 	{
// 		pGroup = *iter;
// 	}
// 
// 	std::vector<MIModelMeshInstance*>::iterator it = find(pGroup->vMeshIns.begin(), pGroup->vMeshIns.end(), pMeshInstance);
// 	if (it != pGroup->vMeshIns.end())
// 		return;
// 
// 	pGroup->vMeshIns.push_back(pMeshInstance);
// }
// 
// void MScene::CancelRecordMeshInstance(MIModelMeshInstance* pMeshInstance)
// {
// 	if (!pMeshInstance->GetMaterial())
// 		return;
// 
// 	MMaterial* pMaterial = pMeshInstance->GetMaterial();
// 	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vRenderMeshGroup.begin(), m_vRenderMeshGroup.end(), pMaterial, [](MaterialMeshInsGroup* a, MMaterial* b) {return a->pMat < b; });
// 	if (iter == m_vRenderMeshGroup.end())
// 		return;
// 	
// 	MaterialMeshInsGroup* pGroup = *iter;
// 
// 	std::vector<MIModelMeshInstance*>::iterator it = find(pGroup->vMeshIns.begin(), pGroup->vMeshIns.end(), pMeshInstance);
// 	if (it != pGroup->vMeshIns.end())
// 		pGroup->vMeshIns.erase(it);
// 
// 	if (pGroup->vMeshIns.empty())
// 	{
// 		m_vRenderMeshGroup.erase(iter);
// 		delete pGroup;
// 		pGroup = nullptr;
// 	}
// }