#include "MScene.h"
#include "MFunction.h"
#include "MEngine.h"
#include "MSkyBox.h"

#include "MIDevice.h"

#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MNode.h"
#include "MVertex.h"
#include "MMaterial.h"
#include "MIRenderer.h"
#include "MIRenderView.h"
#include "MViewport.h"
#include "MTransformCoord.h"
#include "MBounds.h"
#include "Model/MModelResource.h"
#include "MPainter.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"
#include "MTexture.h"

#include "MSkeleton.h"
#include "Texture/MTextureResource.h"

#if MORTY_RENDER_DATA_STATISTICS
#include "MRenderStatistics.h"
#endif

#include "MSceneComponent.h"
#include "MInputComponent.h"
#include "MSpotLightComponent.h"
#include "MPointLightComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

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

	if (m_pTransformCoord3D)
	{
		m_pTransformCoord3D->DeleteLater();
		m_pTransformCoord3D = nullptr;
	}

	if (m_pRootNode)
	{
		MNode* pDeletedNode = m_pRootNode;
		pDeletedNode->SetAttachedScene(nullptr);
		pDeletedNode->DeleteLater();

		m_pRootNode = nullptr;
	}

	Super::OnDelete();
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

MDirectionalLightComponent* MScene::FindActiveDirectionLight()
{
	MComponentGroup* pComponentGroup = FindComponents<MDirectionalLightComponent>();
	if (!pComponentGroup || pComponentGroup->m_vComponent.empty())
		return nullptr;

	return static_cast<MDirectionalLightComponent*>(pComponentGroup->m_vComponent.back());
}

void MScene::FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLightComponent*>& vPointLights)
{
	MComponentGroup* pComponentGroup = FindComponents<MPointLightComponent>();
	if (!pComponentGroup)
		return;

	static auto compareFunc = [v3WorldPosition](MComponent* a, MComponent* b) {
		MSceneComponent* pComponentA = a->GetOwnerNode()->GetComponent<MSceneComponent>();
		MSceneComponent* pComponentB = b->GetOwnerNode()->GetComponent<MSceneComponent>();

		if (!pComponentA) return true;
		if (!pComponentB) return false;

		return (pComponentA->GetWorldPosition() - v3WorldPosition).Length() < (pComponentB->GetWorldPosition() - v3WorldPosition).Length();
	};

	if (pComponentGroup->m_vComponent.size() <= vPointLights.size())
	{
		for (size_t i = 0; i < pComponentGroup->m_vComponent.size(); ++i)
		{
			vPointLights[i] = static_cast<MPointLightComponent*>(pComponentGroup->m_vComponent[i]);
		}
	}
	else if (vPointLights.size() * 2 < pComponentGroup->m_vComponent.size())
	{
		std::fill(vPointLights.begin(), vPointLights.end(), nullptr);
		for (MComponent* pComponent : pComponentGroup->m_vComponent)
		{
			MPointLightComponent* pPointLightComponent = static_cast<MPointLightComponent*>(pComponent);
			for (auto iter = vPointLights.begin(); iter != vPointLights.end(); ++iter)
			{
				if (*iter == nullptr)
				{
					(*iter) = pPointLightComponent;
					break;
				}
				else if (compareFunc(pPointLightComponent, *iter))
				{
					for (auto nextIter = vPointLights.end() - 1; nextIter != iter; --nextIter)
						(*nextIter) = *(nextIter - 1);
					(*iter) = pPointLightComponent;
					break;
				}
			}
		}
	}
	else
	{
		std::sort(pComponentGroup->m_vComponent.begin(), pComponentGroup->m_vComponent.end(), compareFunc);

		for (size_t i = 0; i < vPointLights.size(); ++i)
		{
			vPointLights[i] = (static_cast<MPointLightComponent*>(pComponentGroup->m_vComponent[i]));
		}
	}
}

void MScene::FindActiveSpotLights(const Vector3& v3WorldPosition, std::vector<MSpotLightComponent*>& vSpotLights)
{
	MComponentGroup* pComponentGroup = FindComponents<MSpotLightComponent>();
	if (!pComponentGroup)
		return;

	for (uint32_t i = 0; i < vSpotLights.size() && i < pComponentGroup->m_vComponent.size(); ++i)
	{
		vSpotLights[i] = static_cast<MSpotLightComponent*>(pComponentGroup->m_vComponent[i]);
	}
}

void MScene::OnNodeEnter(MNode* pNode)
{
	auto& vComponents = pNode->GetComponents();

	for (MComponent* pComponent : vComponents)
	{
		AddComponent(pComponent);
		pComponent->OnEnterScene(this);
	}
}

void MScene::OnNodeExit(MNode* pNode)
{
	if (m_pRootNode == pNode)
		m_pRootNode = nullptr;

	auto& vComponents = pNode->GetComponents();

	for (MComponent* pComponent : vComponents)
	{
		RemoveComponent(pComponent);
		pComponent->OnExitScene(this);
	}
}

void MScene::AddComponent(MComponent* pComponent)
{
	if (!pComponent)
		return;

	if(MRenderableMeshComponent* pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
		InsertMaterialGroup(pMeshComponent);

	MTypeIdentifierConstPointer pCmoponentType = pComponent->GetTypeIdentifier();

	while (pCmoponentType && pCmoponentType != MComponent::GetClassTypeIdentifier())
	{
		MComponentGroup* pCompGroup = nullptr;
		auto findResult = m_tComponents.find(pCmoponentType);
		if (findResult == m_tComponents.end())
		{
			pCompGroup = new MComponentGroup();
			m_tComponents[pCmoponentType] = pCompGroup;
		}
		else
		{
			pCompGroup = findResult->second;
		}

		if (pCompGroup)
		{
			pCompGroup->AddComponent(pComponent);
		}

		pCmoponentType = pCmoponentType->m_pBaseTypeIdentifier;
	}
}

void MScene::RemoveComponent(MComponent* pComponent)
{
	if (!pComponent)
		return;

	if (MRenderableMeshComponent* pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
		RemoveMaterialGroup(pMeshComponent);

	MTypeIdentifierConstPointer pCmoponentType = pComponent->GetTypeIdentifier();

	while (pCmoponentType && pCmoponentType != MComponent::GetClassTypeIdentifier())
	{
		auto findResult = m_tComponents.find(pCmoponentType);

		MComponentGroup* pCompGroup = findResult->second;

		if (pCompGroup)
		{
			pCompGroup->RemoveComponent(pComponent);
		}

		pCmoponentType = pCmoponentType->m_pBaseTypeIdentifier;
	}
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

	auto findResult = m_tComponents.find(MInputComponent::GetClassTypeIdentifier());
	if (findResult == m_tComponents.end())
		return;

	for (MComponent* pComponent : findResult->second->m_vComponent)
	{
		if (MInputComponent* pInputComponent = static_cast<MInputComponent*>(pComponent))
		{
			if (pInputComponent->Input(pEvent, pViewport))
				break;
		}
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



void MScene::InsertMaterialGroup(MRenderableMeshComponent* pMeshComponent)
{
	if (!pMeshComponent)
		return;

	MMaterial* pMaterial = pMeshComponent->GetMaterial();
	std::vector<MMaterialGroup*>::iterator iter = std::lower_bound(m_vMaterialGroups.begin(), m_vMaterialGroups.end(), pMaterial, [](MMaterialGroup* a, MMaterial* b) {return a->m_pMaterial < b; });
	MMaterialGroup* pGroup = nullptr;
	if (iter == m_vMaterialGroups.end())
	{
		pGroup = new MMaterialGroup();
		pGroup->m_pMaterial = pMeshComponent->GetMaterial();
		m_vMaterialGroups.push_back(pGroup);
	}
	else if ((*iter)->m_pMaterial != pMaterial)
	{
		pGroup = new MMaterialGroup();
		pGroup->m_pMaterial = pMeshComponent->GetMaterial();
		m_vMaterialGroups.insert(iter, pGroup);
	}
	else
	{
		pGroup = *iter;
	}

	pGroup->InsertMeshComponent(pMeshComponent);
}

void MScene::RemoveMaterialGroup(MRenderableMeshComponent* pMeshComponent)
{
	if (!pMeshComponent)
		return;

	MMaterial* pMaterial = pMeshComponent->GetMaterial();
	std::vector<MMaterialGroup*>::iterator iter = std::lower_bound(m_vMaterialGroups.begin(), m_vMaterialGroups.end(), pMaterial, [](MMaterialGroup* a, MMaterial* b) {return a->m_pMaterial < b; });
	if (iter == m_vMaterialGroups.end())
		return;

	MMaterialGroup* pGroup = *iter;
	pGroup->RemoveMeshComponent(pMeshComponent);

	if (pGroup->m_vMeshComponents.empty())
	{
		m_vMaterialGroups.erase(iter);
		delete pGroup;
		pGroup = nullptr;
	}
}

void MComponentGroup::AddComponent(MComponent* pComponent)
{
	UNION_PUSH_BACK_VECTOR(m_vComponent, pComponent);
}

void MComponentGroup::RemoveComponent(MComponent* pComponent)
{
	ERASE_FIRST_VECTOR(m_vComponent, pComponent);
}
