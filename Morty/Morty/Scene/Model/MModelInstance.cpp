#include "MModelInstance.h"
#include "MModelResource.h"
#include "MMeshInstance.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"

#include "MEngine.h"

MModelInstance::MModelInstance()
	: m_pResource(nullptr)
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
}

bool MModelInstance::Load(MResource* pResource)
{
	if (MModelResource* pModelRes = dynamic_cast<MModelResource*>(pResource))
	{
		m_pResource = pModelRes;

		int index = 0;
		char svIndexx[16];
		for (MIMesh* pMesh : *pModelRes->GetMeshes())
		{
			MMeshInstance* pMeshIns = GetObjectManager()->CreateObject<MMeshInstance>();
			pMeshIns->SetMesh(pMesh);
			itoa(index, svIndexx, 10);
			pMeshIns->SetName(MString("Mesh_") + svIndexx);
			AddNode(pMeshIns);
		}

		m_pSkeleton = new MSkeletonInstance(pModelRes->GetSkeleton());

		//Do something.
		return true;
	}

	return false;
}

bool MModelInstance::SetPlayAnimation(const MString& strAnimationName)
{
	if (m_pCurrentAnimationController)
	{
		delete m_pCurrentAnimationController;
		m_pCurrentAnimationController = nullptr;
	}

	MSkeletalAnimation* pAnimation = (*m_pResource->GetAnimations()).at(strAnimationName);

	MSkeletalAnimController* pController = new MSkeletalAnimController();
	if (pController->Initialize(m_pSkeleton, pAnimation))
	{
		m_pCurrentAnimationController = pController;

		//Test
		pController->SetLoop(true);
		pController->Play();

		return true;
	}


	return false;
}

void MModelInstance::Tick(const float& fDelta)
{
	M3DNode::Tick(fDelta);

	if (m_pCurrentAnimationController)
	{
		m_pCurrentAnimationController->Update(fDelta);
	}
}
