#include "MModelMeshData.h"
#include "MModelResource.h"
#include "MEngine.h"
#include "MMeshDetailMap.h"

MModelMeshData::MModelMeshData()
	: m_eVertexType(MEMeshVertexType::Normal)
	, m_pMesh(nullptr)
	, m_matRotationMatrix(Matrix4::IdentityMatrix)
	, m_pBoundsOBB(nullptr)
	, m_pMaterial(nullptr)
	, m_pResource(nullptr)
	, m_pMeshDetailMap(nullptr)
{
	if (m_pMeshDetailMap)
	{
		delete m_pMeshDetailMap;
		m_pMeshDetailMap = nullptr;
	}
}

MModelMeshData::~MModelMeshData()
{
}

MIMesh* MModelMeshData::GetLevelMesh(const unsigned int unLevel)
{
	if (m_pMesh)
	{
		if (nullptr == m_pMeshDetailMap)
		{
			m_pMeshDetailMap = new MMeshDetailMap();
			m_pMeshDetailMap->BindMesh(m_pMesh);
		}

		return m_pMeshDetailMap->GetLevel(unLevel);
	}

	return nullptr;
}
