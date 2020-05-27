#include "MModelInstance.h"
#include "Model/MModelMeshStruct.h"
#include "Model/MModelResource.h"
#include "MStaticMeshInstance.h"
#include "MSkinnedMeshInstance.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"
#include "MBounds.h"

#include "MEngine.h"

M_OBJECT_IMPLEMENT(MModelInstance, M3DNode)

MModelInstance::MModelInstance()
	: M3DNode()
	, m_pBoundsAABB(nullptr)
	, m_ModelResource(nullptr)
	, m_pSkeleton(nullptr)
	, m_pCurrentAnimationController(nullptr)
	, m_bDrawBoundingBox(false)
	, m_bGenerateDirLightShadow(false)
{

}

MModelInstance::~MModelInstance()
{
	if (m_pSkeleton)
	{
		delete m_pSkeleton;
		m_pSkeleton = nullptr;
	}

	if (m_pCurrentAnimationController)
	{
		delete m_pCurrentAnimationController;
		m_pCurrentAnimationController = nullptr;
	}

	m_ModelResource.SetResource(nullptr);

	if (m_pBoundsAABB)
	{
		delete m_pBoundsAABB;
		m_pBoundsAABB = nullptr;
	}
}

bool MModelInstance::Load(MResource* pResource)
{
	if (MModelResource* pModelRes = dynamic_cast<MModelResource*>(pResource))
	{
		auto UseResourceFunction = [this](const unsigned int& eReloadType) {
			if (MModelResource* pModelResource = static_cast<MModelResource*>(m_ModelResource.GetResource()))
			{
				ClearSkeletonAndMesh();

				if (!pModelResource->GetSkeleton()->GetAllBones().empty())
				{
					//Create SkeletonInstance
					m_pSkeleton = new MSkeletonInstance(pModelResource->GetSkeleton());
				}

				//初始化Mesh的旋转矩阵
				for (MModelMeshStruct* pMeshData : *pModelResource->GetMeshes())
				{
					MIModelMeshInstance* pMeshIns = nullptr;
					if (pMeshData->GetMeshVertexType() == MModelMeshStruct::Normal)
					{
						MStaticMeshInstance* pStaticMeshIns = GetObjectManager()->CreateObject<MStaticMeshInstance>();
						pStaticMeshIns->SetMeshData(pMeshData);
						pMeshIns = pStaticMeshIns;
					}
					else
					{
						MSkinnedMeshInstance* pSkinnedMeshIns = GetObjectManager()->CreateObject<MSkinnedMeshInstance>();
						pSkinnedMeshIns->SetSkeletonInstance(m_pSkeleton);
						pSkinnedMeshIns->SetMeshData(pMeshData);
						pMeshIns = pSkinnedMeshIns;
					}
						
					pMeshIns->SetRotation(pMeshData->GetMeshesRotationMatrix()->GetRotation());

					pMeshIns->SetName(pMeshData->GetMeshName());
					pMeshIns->SetAttachedModelInstance(this);

					AddNodeImpl(pMeshIns, MENodeChildType::EFixed);
				}
			}
	
			return true;
		};

		m_ModelResource.SetResource(pModelRes);
		m_ModelResource.SetResChangedCallback(UseResourceFunction);

		UseResourceFunction(MResource::EResReloadType::EDefault);

		return true;
	}

	return false;
}

void MModelInstance::ClearSkeletonAndMesh()
{
	SetRemoveAnimation();
	if (m_pSkeleton)
	{
		delete m_pSkeleton;
		m_pSkeleton = nullptr;
	}
	RemoveAllNodeImpl(MENodeChildType::EFixed);
}

void MModelInstance::Unload()
{
	ClearSkeletonAndMesh();

	m_ModelResource.SetResource(nullptr);
}

MModelResource* MModelInstance::GetResource()
{
	return static_cast<MModelResource*>(m_ModelResource.GetResource());
}

MBoundsAABB* MModelInstance::GetBoundsAABB()
{
	Vector3 v3Min, v3Max;
	v3Min = v3Max = GetWorldPosition();
	
	for (MNode* pNode : m_vFixedChildren)
	{
		if (MIMeshInstance* pMeshIns = dynamic_cast<MIMeshInstance*>(pNode))
		{
			pMeshIns->GetBoundsAABB()->UnionMinMax(v3Min, v3Max);
		}
	}

	if (nullptr == m_pBoundsAABB)
		m_pBoundsAABB = new MBoundsAABB();

	m_pBoundsAABB->SetMinMax(v3Min, v3Max);
	return m_pBoundsAABB;
}

bool MModelInstance::SetPlayAnimation(const MString& strAnimationName)
{
	SetRemoveAnimation();

	MSkeletalAnimation* pAnimation = (*static_cast<MModelResource*>(m_ModelResource.GetResource())->GetAnimations()).at(strAnimationName);

	MSkeletalAnimController* pController = new MSkeletalAnimController();
	if (pController->Initialize(m_pSkeleton, pAnimation))
	{
		m_pCurrentAnimationController = pController;

		return true;
	}

	return false;
}

void MModelInstance::SetRemoveAnimation()
{
	if (m_pCurrentAnimationController)
	{
		delete m_pCurrentAnimationController;
		m_pCurrentAnimationController = nullptr;
	}
}

void MModelInstance::Tick(const float& fDelta)
{
	M3DNode::Tick(fDelta);

	if (m_pCurrentAnimationController && m_pCurrentAnimationController->GetState() == MIAnimController::EPlay)
	{
		m_pCurrentAnimationController->Update(fDelta, GetVisibleRecursively());
	}
}

void MModelInstance::OnDelete()
{
	Unload();


	Super::OnDelete();
}

void MModelInstance::SetVisible(const bool& bVisible)
{
	M3DNode::SetVisible(bVisible);
}
