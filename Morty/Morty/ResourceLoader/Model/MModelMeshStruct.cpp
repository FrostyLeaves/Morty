#include "MModelMeshStruct.h"
#include "MModelResource.h"
#include "MEngine.h"
#include "MMultiLevelMesh.h"

MModelMeshStruct::MModelMeshStruct()
	: m_eVertexType(MEMeshVertexType::Normal)
	, m_pMesh(nullptr)
	, m_matRotationMatrix(Matrix4::IdentityMatrix)
	, m_pBoundsOBB(nullptr)
	, m_MaterialKeeper()
	, m_unMeshIndex(0)
	, m_pResource(nullptr)
	, m_pMeshDetailMap(nullptr)
{
	
}

MModelMeshStruct::~MModelMeshStruct()
{
	if (m_pMeshDetailMap)
	{
		delete m_pMeshDetailMap;
		m_pMeshDetailMap = nullptr;
	}
}

MIMesh* MModelMeshStruct::GetLevelMesh(const unsigned int unLevel)
{
	if (m_pMesh)
	{
		if (nullptr == m_pMeshDetailMap)
		{
			m_pMeshDetailMap = new MMultiLevelMesh();
			m_pMeshDetailMap->BindMesh(m_pMesh);
		}

		return m_pMeshDetailMap->GetLevel(unLevel);
	}

	return nullptr;
}
