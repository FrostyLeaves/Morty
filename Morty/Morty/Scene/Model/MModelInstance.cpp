#include "MModelInstance.h"
#include "MModelResource.h"
#include "MStaticMeshInstance.h"
#include "MSkinnedMeshInstance.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"
#include "MBounds.h"

#include "MEngine.h"

MTypeIdentifierImplement(MModelInstance, M3DNode)

MModelInstance::MModelInstance()
	: M3DNode()
	, m_pBoundsAABB(nullptr)
	, m_pModelResource(nullptr)
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

	if (m_pModelResource)
	{
		delete m_pModelResource;
		m_pModelResource = nullptr;
	}

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
			if (MModelResource* pModelResource = static_cast<MModelResource*>(m_pModelResource->GetResource()))
			{
				SetRemoveAnimation();
				if (m_pSkeleton)
				{
					delete m_pSkeleton;
					m_pSkeleton = nullptr;
				}
				RemoveAllNodeImpl(MENodeChildType::EFixed);

				if (!pModelResource->GetSkeleton()->GetAllBones().empty())
				{
					//Create SkeletonInstance
					m_pSkeleton = new MSkeletonInstance(pModelResource->GetSkeleton());
				}

				int index = 0;

				//初始化Mesh的旋转矩阵
				const std::vector<Matrix4>& vMeshesRotationMatrix = *pModelResource->GetMeshesRotationMatrix();
				const std::vector<MBoundsOBB*>& vMeshesDefaultOBB = *pModelResource->GetMeshesDefaultOBB();
				for (MIMesh* pMesh : *pModelResource->GetMeshes())
				{
					MIMeshInstance* pMeshIns = nullptr;
					if (pModelResource->GetMeshVertexType(index) == MModelResource::Normal)
					{
						MStaticMeshInstance* pStaticMeshIns = GetObjectManager()->CreateObject<MStaticMeshInstance>();
						pStaticMeshIns->SetDefaultOBB(vMeshesDefaultOBB[index]);
						pMeshIns = pStaticMeshIns;
					}
					else
					{
						MSkinnedMeshInstance* pSkinnedMeshIns = GetObjectManager()->CreateObject<MSkinnedMeshInstance>();
						pSkinnedMeshIns->SetDefaultOBB(vMeshesDefaultOBB[index]);
						pSkinnedMeshIns->SetSkeletonInstance(m_pSkeleton);
						pMeshIns = pSkinnedMeshIns;
					}
						
					pMeshIns->SetMesh(pMesh);
					pMeshIns->SetRotation(vMeshesRotationMatrix[index].GetRotation());

					pMeshIns->SetName(MString("Mesh_") + MStringHelper::ToString(index));
					pMeshIns->SetMaterial(pModelResource->GetMeshDefaultMaterial(index));
					pMeshIns->SetAttachedModelInstance(this);

					AddNodeImpl(pMeshIns, MENodeChildType::EFixed);

					++index;
				}
			}
	
			return true;
		};

		MResourceHolder* pNewHolder = new MResourceHolder(pModelRes);
		pNewHolder->SetResChangedCallback(UseResourceFunction);

		if (m_pModelResource)
			delete m_pModelResource;

		m_pModelResource = pNewHolder;

		UseResourceFunction(MResource::EResReloadType::EDefault);

		return true;
	}

	return false;
}

MModelResource* MModelInstance::GetResource()
{
	if (nullptr == m_pModelResource)
		return nullptr;
	return static_cast<MModelResource*>(m_pModelResource->GetResource());
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

	MSkeletalAnimation* pAnimation = (*static_cast<MModelResource*>(m_pModelResource->GetResource())->GetAnimations()).at(strAnimationName);

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

void MModelInstance::SetVisible(const bool& bVisible)
{
	M3DNode::SetVisible(bVisible);
}
