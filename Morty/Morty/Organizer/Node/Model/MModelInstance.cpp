#include "MModelInstance.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"
#include "Model/MSkeletalAnimationResource.h"
#include "MStaticMeshInstance.h"
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
	SetSkeletonTemplate(nullptr);

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
	SetSkeletonTemplate(nullptr);
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
	if (MResource* pAnimResource = GetEngine()->GetResourceManager()->LoadResource(strAnimationName))
	{
		return SetPlayAnimation(pAnimResource);
	}

	return false;
}

bool MModelInstance::SetPlayAnimation(MResource* pAnimation)
{
	SetRemoveAnimation();

	if (MSkeletalAnimationResource* pAnimRes = pAnimation->DynamicCast<MSkeletalAnimationResource>())
	{
		MSkeletalAnimController* pController = new MSkeletalAnimController();
		if (pController->Initialize(m_pSkeleton, pAnimRes))
		{
			m_pCurrentAnimationController = pController;

			return true;
		}
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
	Super::Tick(fDelta);

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
	M_SERIALIZER_WRITE_VALUE("SkeletonResource", GetSkeletonTemplatePath)

	M_SERIALIZER_END;
}

void MModelInstance::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_BEGIN(Read);
	M_SERIALIZER_READ_VALUE("SkeletonResource", SetSkeletonTemplatePath, String);

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
		auto UseResourceFunction = [this](const uint32_t& eReloadType) {
			if (MModelResource* pModelResource = static_cast<MModelResource*>(m_ModelResource.GetResource()))
			{
				ClearSkeletonAndMesh();

				//Create SkeletonInstance
				SetSkeletonTemplate(pModelResource->GetSkeleton());

				//初始化Mesh的旋转矩阵
				for (MMeshResource* pMeshResource : pModelResource->GetMeshResources())
				{
					MStaticMeshInstance* pMeshIns = GetObjectManager()->CreateObject<MStaticMeshInstance>();
					pMeshIns->Load(pMeshResource);
	

					//pMeshIns->SetRotation(pMeshResource->GetMeshesRotationMatrix()->GetRotation());

					pMeshIns->SetName(pMeshResource->GetMeshName());
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
			if (nullptr == m_pSkeleton || m_pSkeleton->GetSkeletonTemplate() != pModelRes->GetSkeleton())
				SetSkeletonTemplate(pModelRes->GetSkeleton());
		}

		return true;
	}

	return false;
}

void MModelInstance::SetSkeletonTemplate(MSkeleton* pSkeleton)
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


	m_SkeletonResource.SetResource(pSkeleton);
}

void MModelInstance::SetSkeletonTemplatePath(const MString& strSkeletonPath)
{
	if (MResource* pResource = GetEngine()->GetResourceManager()->LoadResource(strSkeletonPath))
	{
		SetSkeletonTemplate(pResource->DynamicCast<MSkeletonResource>());
	}
}

MString MModelInstance::GetSkeletonTemplatePath() const
{
	return m_SkeletonResource.GetResourcePath();
}

