#include "MModelInstance.h"
#include "MModelResource.h"
#include "MStaticMeshInstance.h"
#include "MSkinnedMeshInstance.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"

#include "MEngine.h"

MModelInstance::MModelInstance()
	: m_pModelResource(nullptr)
	, m_pSkeleton(nullptr)
	, m_pCurrentAnimationController(nullptr)
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
}

bool MModelInstance::Load(MResource* pResource)
{
	if (MModelResource* pModelRes = dynamic_cast<MModelResource*>(pResource))
	{
		auto UseResourceFunction = [this]() {
			if (MModelResource* pModelResource = static_cast<MModelResource*>(m_pModelResource->GetResource()))
			{
				SetRemoveAnimation();
				if (m_pSkeleton)
				{
					delete m_pSkeleton;
					m_pSkeleton = nullptr;
				}
				RemoveAllNodeImpl(MENodeChildType::MEFixed);
				

				int index = 0;
				for (MIMesh* pMesh : *pModelResource->GetMeshes())
				{
					MIMeshInstance* pMeshIns = nullptr;
					if (pModelResource->GetMeshVertexType(index) == MModelResource::Normal)
						pMeshIns = GetObjectManager()->CreateObject<MStaticMeshInstance>();
					else
						pMeshIns = GetObjectManager()->CreateObject<MSkinnedMeshInstance>();

					pMeshIns->SetMesh(pMesh);
					pMeshIns->SetName(MString("Mesh_") + MStringHelper::ToString(index));
					AddNodeImpl(pMeshIns, MENodeChildType::MEFixed);
					//AddNodeImpl(pMeshIns, MENodeChildType::MENormal);

					pMeshIns->SetMaterial(pModelResource->GetMeshDefaultMaterial(index));

					++index;
				}

				m_pSkeleton = new MSkeletonInstance(pModelResource->GetSkeleton());
			}
	
		};

		m_pModelResource = new MResourceHolder(pModelRes);
		m_pModelResource->SetResChangedCallback(UseResourceFunction);

		UseResourceFunction();

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

	if (m_pCurrentAnimationController)
	{
		m_pCurrentAnimationController->Update(fDelta);
	}
}
