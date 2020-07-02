#include "MScene.h"
#include "MFunction.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"

#include "MIDevice.h"

#include "Light/MDirectionalLight.h"
#include "Light/MPointLight.h"
#include "Light/MSpotLight.h"

#include "Model/MMeshResource.h"
#include "Model/MIModelMeshInstance.h"
#include "Model/MStaticMeshInstance.h"
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
#include "MTransparentRenderTarget.h"

#include <algorithm>

M_OBJECT_IMPLEMENT(MScene, MObject)


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
	, m_vViewports()
{
	
}

void MScene::OnCreated()
{
	MObject::OnCreated();

	m_pSkyBox = m_pEngine->GetObjectManager()->CreateObject<MSkyBox>();
	m_pTransformCoord3D = m_pEngine->GetObjectManager()->CreateObject<MTransformCoord3D>();

}

void MScene::OnDelete()
{
	if (m_pSkyBox)
	{
		m_pSkyBox->DeleteLater();
		m_pSkyBox = nullptr;
	}


	Super::OnDelete();
}

void MScene::CleanAllNodes()
{

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

	for (uint32_t i = 0; i < vSpotLights.size() && i < m_vSpotLight.size(); ++i)
	{
		vSpotLights[i] = m_vSpotLight[i];
	}
}

void MScene::OnNodeEnter(MNode* pNode)
{
 	if (MIMeshInstance* pMesh = pNode->DynamicCast<MIMeshInstance>())
		InsertMaterialGroup(pMesh);

	else if (MCamera* pCamera = pNode->DynamicCast<MCamera>())
	{
		for (MViewport* pViewport : m_vViewports)
		{
			if (pViewport->IsUseDefaultCamera())
				pViewport->SetCamera(pCamera);
		}
	}

MSCENE_ON_NODE_ENTER(ModelInstance)
MSCENE_ON_NODE_ENTER(DirectionalLight)
MSCENE_ON_NODE_ENTER(PointLight)
MSCENE_ON_NODE_ENTER(SpotLight)
MSCENE_ON_NODE_ENTER(InputNode)
}

void MScene::OnNodeExit(MNode* pNode)
{
	if (m_pRootNode == pNode)
		m_pRootNode = nullptr;

 	if (MIMeshInstance* pMesh = pNode->DynamicCast<MIMeshInstance>())
		RemoveMaterialGroup(pMesh);

	else if (MCamera* pCamera = pNode->DynamicCast<MCamera>())
	{
		for (MViewport* pViewport : m_vViewports)
		{
			if (pViewport->GetCamera() == pCamera)
				pViewport->SetCamera(nullptr);
		}
	}

MSCENE_ON_NODE_EXIT(ModelInstance)
MSCENE_ON_NODE_EXIT(DirectionalLight)
MSCENE_ON_NODE_EXIT(PointLight)
MSCENE_ON_NODE_EXIT(SpotLight)
MSCENE_ON_NODE_EXIT(InputNode)
}

void MScene::Render(MIRenderer* pRenderer, MViewport* pViewport, MIRenderTarget* pRenderTarget)
{

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
	
}

void MScene::SetRootNode(MNode* pRootNode)
{
	if (m_pRootNode == pRootNode)
		return;

	if (m_pRootNode)
	{
		m_pRootNode->SetAttachedScene(nullptr);
	}
;
	if (m_pRootNode = pRootNode)
	{
		pRootNode->SetAttachedScene(this);
	}
}



void MScene::InsertMaterialGroup(MIMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	MMaterial* pMaterial = pMeshInstance->GetMaterial();
	std::vector<MMaterialGroup*>::iterator iter = std::lower_bound(m_vMaterialGroups.begin(), m_vMaterialGroups.end(), pMaterial, [](MMaterialGroup* a, MMaterial* b) {return a->m_pMaterial < b; });
	MMaterialGroup* pGroup = nullptr;
	if (iter == m_vMaterialGroups.end())
	{
		pGroup = new MMaterialGroup();
		pGroup->m_pMaterial = pMeshInstance->GetMaterial();
		m_vMaterialGroups.push_back(pGroup);
	}
	else if ((*iter)->m_pMaterial != pMaterial)
	{
		pGroup = new MMaterialGroup();
		pGroup->m_pMaterial = pMeshInstance->GetMaterial();
		m_vMaterialGroups.insert(iter, pGroup);
	}
	else
	{
		pGroup = *iter;
	}

	pGroup->m_vMeshInstances.push_back(pMeshInstance);
}

void MScene::RemoveMaterialGroup(MIMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	MMaterial* pMaterial = pMeshInstance->GetMaterial();
	std::vector<MMaterialGroup*>::iterator iter = std::lower_bound(m_vMaterialGroups.begin(), m_vMaterialGroups.end(), pMaterial, [](MMaterialGroup* a, MMaterial* b) {return a->m_pMaterial < b; });
	if (iter == m_vMaterialGroups.end())
		return;

	MMaterialGroup* pGroup = *iter;
	ERASE_FIRST_VECTOR(pGroup->m_vMeshInstances, pMeshInstance);

	if (pGroup->m_vMeshInstances.empty())
	{
		m_vMaterialGroups.erase(iter);
		delete pGroup;
		pGroup = nullptr;
	}
}
