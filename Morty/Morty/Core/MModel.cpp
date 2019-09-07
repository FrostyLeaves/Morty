#include "MModel.h"

MModel::MModel()
{
    
}

MModel::~MModel()
{
	Clean();
}

void MModel::Clean()
{
	for (MIMesh* pMesh : m_vMeshes)
		delete pMesh;

	m_vMeshes.clear();
}
