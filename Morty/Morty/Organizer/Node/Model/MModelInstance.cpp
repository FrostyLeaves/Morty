#include "MModelInstance.h"
#include "Model/MModelMeshStruct.h"
#include "Model/MModelResource.h"
#include "MStaticMeshInstance.h"
#include "MSkinnedMeshInstance.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"
#include "MBounds.h"

#include "MEngine.h"
#include "MResourceManager.h"

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
	SetSkeleton(nullptr);

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
	
	return SetResource(pResource, true);

}

void MModelInstance::ClearSkeletonAndMesh()
{
	SetRemoveAnimation();
	SetSkeleton(nullptr);
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
	Super::SetVisible(bVisible);
}

void MModelInstance::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_BEGIN(Write);
	M_SERIALIZER_WRITE_VALUE("Resource", GetResourcePath)

	M_SERIALIZER_END;
}

void MModelInstance::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_BEGIN(Read);
	M_SERIALIZER_READ_VALUE("Resource", SetResourcePath, String);

	M_SERIALIZER_END;
}

bool MModelInstance::SetResourcePath(const MString& strResourcePath, const bool& bLoad /*= false*/)
{
	if (MResource* pResource = m_pEngine->GetResourceManager()->LoadResource(strResourcePath))
		return SetResource(pResource, bLoad);
	
	return false;
}

bool MModelInstance::SetResource(MResource* pResource, const bool& bLoad)
{
	if (MModelResource* pModelRes = dynamic_cast<MModelResource*>(pResource))
	{
		auto UseResourceFunction = [this](const unsigned int& eReloadType) {
			if (MModelResource* pModelResource = static_cast<MModelResource*>(m_ModelResource.GetResource()))
			{
				ClearSkeletonAndMesh();

				//Create SkeletonInstance
				SetSkeleton(pModelResource->GetSkeleton());

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

		if (bLoad)
		{
			UseResourceFunction(MResource::EResReloadType::EDefault);
		}
		else
		{
			if (m_pSkeleton->GetSkeletonTemplate() != pModelRes->GetSkeleton())
				SetSkeleton(pModelRes->GetSkeleton());
		}

		return true;
	}

	return false;
}

void MModelInstance::SetSkeleton(const MSkeleton* pSkeleton)
{
	if (m_pSkeleton)
	{
		delete m_pSkeleton;
		m_pSkeleton = nullptr;
	}

	if (pSkeleton)
	{
		m_pSkeleton = new MSkeletonInstance(pSkeleton);
	}
}
