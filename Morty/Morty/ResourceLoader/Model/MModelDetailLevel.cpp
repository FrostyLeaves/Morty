#include "MModelDetailLevel.h"
#include "MModelResource.h"
#include "MLogManager.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b) )
#define MAX(a, b) ((a) > (b) ? (a) : (b) )

int testa = 0;
int testb = 0;


MModelLodFactory::MModelLodFactory()
	: m_pResource(nullptr)
	, m_bInitialized(false)
{

}

MModelLodFactory::~MModelLodFactory()
{
	if (m_pResource)
	{
		delete m_pResource;
	}
}

void MModelLodFactory::Load(MModelResource* pModelResource)
{
	if (m_pResource)
		delete m_pResource;

	auto LoadFunc = [this](const unsigned int& eReloadType) {
		m_bInitialized = false;
		return true;
	};

	m_pResource = new MResourceHolder(pModelResource);
	m_pResource->SetResChangedCallback(LoadFunc);

	LoadFunc(MResource::EResReloadType::EDefault);
}

bool MModelLodFactory::InitWithResource()
{
	if (m_bInitialized)
		return true;

	// TODO

	MModelResource* pResource = m_pResource->GetResource<MModelResource>();
	
	const std::vector<MIMesh*>* pMeshes = pResource->GetMeshes();
	m_vMeshDetailLevel.resize(pMeshes->size());
	for (unsigned int i = 0; i < pMeshes->size(); ++i)
	{
		m_vMeshDetailLevel[i] = new MMeshDetailMap((*pMeshes)[i]);
	}

	m_bInitialized = true;
	return true;
}

MIMesh* MModelLodFactory::CreateLevel(const unsigned int& unIndex, const unsigned int& unPercent)
{
	if (false == m_bInitialized && false == InitWithResource())
		return nullptr;

	std::vector<MIMesh*>& vCahce = m_vCacheMeshes[unIndex];
	if (vCahce.size() > unPercent && vCahce[unPercent] != nullptr)
		return vCahce[unPercent];

	MMeshDetailMap* pDetailMap = m_vMeshDetailLevel[unIndex];
	unsigned int unVertexNumber = (float)unPercent * pDetailMap->m_pMesh->GetVerticesLength();

	MIMesh* pMesh = pDetailMap->CreateLevel(unVertexNumber);
	if (vCahce.size() < unPercent + 1)
		vCahce.resize(unPercent + 1);
	vCahce[unPercent] = pMesh;

	return pMesh;
}

MMeshDetailMap::MMeshDetailMap(const MIMesh* pMesh)
	: m_pMesh(pMesh)
	, m_pSortVertices(nullptr)
	, m_vMeshesCache(10)
{
	const MByte* pVertices = (MByte*)pMesh->GetVertices();
	const unsigned int* vIndices = pMesh->GetIndices();
	unsigned int unVertexSize = pMesh->GetVertexStructSize();
	unsigned int unVerticesLength = pMesh->GetVerticesLength();
	unsigned int unIndicesLength = pMesh->GetIndicesLength();

	m_vIndexToMap.resize(unVerticesLength);
	m_vMap.resize(unVerticesLength);

	std::vector<Vertex*> vVertices(unVerticesLength);

	for (unsigned int i = 0; i < unVerticesLength; ++i)
	{
		Vertex* pVertex = new Vertex();
		pVertex->unVertexIndex = i;
		pVertex->pos = *((Vector3*)((char*)pVertices + i * unVertexSize));
		pVertex->pCollapseVertex = nullptr;
		vVertices[i] = pVertex;
	}

	for (unsigned int i = 0; i < unIndicesLength; i += 3)
	{
		Face* pFace = new Face();
		pFace->vIndices[0] = vVertices[vIndices[i]];
		pFace->vIndices[1] = vVertices[vIndices[i + 1]];
		pFace->vIndices[2] = vVertices[vIndices[i + 2]];
		ComputeNormal(pFace);
		m_vFaces.push_back(pFace);

		for (unsigned int n = i; n < i + 3; ++n)
		{
			Vertex* pVtx1 = vVertices[vIndices[n]];
			UnionPushBack(pVtx1->vFaces, pFace);
			for (unsigned int m = n + 1; m < i + 3; ++m)
			{
				Vertex* pVtx2 = vVertices[vIndices[m]];
				if (pVtx1 != pVtx2)
				{
					UnionPushBack(pVtx1->vNeighbor, pVtx2);
					UnionPushBack(pVtx2->vNeighbor, pVtx1);
				}
			}
		}
	}

	for (Vertex* pVertex : vVertices)
		UpdateCollapse(pVertex);

	int xxx = 0;
	for (Vertex* p : vVertices)
	{
		if (p->vFaces.size() == 1)
			xxx++;
	}

	MLogManager::GetInstance()->Log(" all:  %d  %d", vVertices.size(), xxx);
	while(!vVertices.empty())
	{
		unsigned xxx = 0;
		for (Vertex* pVtx : vVertices)
			if (pVtx->pCollapseVertex)
				++xxx;
		MLogManager::GetInstance()->Log(" count:  %d", xxx);
		Vertex* pVertex = GetMinCollapseCostVertex(vVertices);
		m_vIndexToMap[pVertex->unVertexIndex] = vVertices.size() - 1;
		m_vMap[vVertices.size() - 1] = (pVertex->pCollapseVertex) ? pVertex->pCollapseVertex->unVertexIndex : -1;
		if (pVertex->pCollapseVertex)
			Collapse(pVertex, pVertex->pCollapseVertex);
		
		EraseFirst(vVertices, pVertex);
		Unuse(pVertex);
		delete pVertex;
	}

	for (unsigned int i = 0; i < m_vMap.size(); i++) {
		m_vMap[i] = (m_vMap[i] == -1) ? 0 : m_vIndexToMap[m_vMap[i]];
	}

	unsigned int unMemorySize = pMesh->GetVertexStructSize() * pMesh->GetVerticesLength();
	m_pSortVertices = new MByte[unMemorySize];

	for (unsigned int i = 0; i < unVerticesLength; ++i) {
		memcpy(m_pSortVertices + (m_vIndexToMap[i] * unVertexSize), pVertices + (i * unVertexSize), unVertexSize);
	}
}

MIMesh* MMeshDetailMap::CreateLevel(const unsigned int& unVertexNumber)
{
	const unsigned int* vIndices = m_pMesh->GetIndices();
	unsigned int unVertexSize = m_pMesh->GetVertexStructSize();
	unsigned int unIndicesLength = m_pMesh->GetIndicesLength();

	MIMesh* pMesh = m_pMesh->Copy();
	unsigned int* vNewIndices = pMesh->GetIndices();
	unsigned int ni = 0;


	for (unsigned int i = 0; i < unIndicesLength; i += 3)
	{
		unsigned int unIndex[3];
		for (unsigned int n = 0; n < 3; ++n)
		{
			unIndex[n] = m_vIndexToMap[vIndices[i + n]];
			while (unIndex[n] >= unVertexNumber)
				unIndex[n] = m_vMap[unIndex[n]];
		}

		if(unIndex[0] == unIndex[1] || unIndex[1] == unIndex[2] || unIndex[2] == unIndex[0])
			continue;

		vNewIndices[ni++] = unIndex[0];
		vNewIndices[ni++] = unIndex[1];
		vNewIndices[ni++] = unIndex[2];
	}


	pMesh->ResizeVertices(unVertexNumber);
	pMesh->ResizeIndices(ni, 1);

	memcpy((MByte*)pMesh->GetVertices(), m_pSortVertices, unVertexNumber * unVertexSize);

	return pMesh;
}

MIMesh* MMeshDetailMap::GetLevel(unsigned int unLevel)
{
	if (unLevel < 1) unLevel = 1;
	if (unLevel > 10) unLevel = 10;

	if (m_vMeshesCache[unLevel] == nullptr)
	{
		unsigned int unVertexNumber = m_pMesh->GetVerticesLength() * (float)unLevel / 10;
		m_vMeshesCache[unLevel] = CreateLevel(unVertexNumber);
	}

	return m_vMeshesCache[unLevel];
}

void MMeshDetailMap::Unuse(Vertex* pVertex)
{
	for (Vertex* pNeighbor : pVertex->vNeighbor)
	{
		EraseFirst(pNeighbor->vNeighbor, pVertex);
		EraseFirst(pVertex->vNeighbor, pNeighbor);
	}
}

void MMeshDetailMap::Unuse(Face* pFace)
{
	for (unsigned int i = 0; i < 3; ++i)
		EraseFirst(pFace->vIndices[i]->vFaces, pFace);
	
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = i + 1; j < 3; ++j)
		{
			UpdateNeighbor(pFace->vIndices[i], pFace->vIndices[j]);
		}
	}

	EraseFirst(m_vFaces, pFace);
}

void MMeshDetailMap::Clean()
{

}

bool MMeshDetailMap::HasVertex(Face* pFace, Vertex* pVertex)
{
	return pFace->vIndices[0] == pVertex || pFace->vIndices[1] == pVertex || pFace->vIndices[2] == pVertex;
}

void MMeshDetailMap::ReplaceVertex(Face* pFace, Vertex* pFrom, Vertex* pTo)
{
	//Remove vertex from face
	for (unsigned int i = 0; i < 3; ++i)
	{
		if (pFace->vIndices[i] == pFrom)
			pFace->vIndices[i] = pTo;
	}

	//Remove face from vertex
	EraseFirst(pFrom->vFaces, pFace);

	//Add face to vertex
	UnionPushBack(pTo->vFaces, pFace);

	for (unsigned int i = 0; i < 3; ++i)
	{
		UpdateNeighbor(pFace->vIndices[i], pFrom);
		
		if (pFace->vIndices[i] != pTo)
		{
			UpdateNeighbor(pFace->vIndices[i], pTo);
		}
	}

	//Compute normal.
	ComputeNormal(pFace);
}

void MMeshDetailMap::UpdateNeighbor(Vertex* pVtx1, Vertex* pVtx2)
{
	for (Face* pFace : pVtx1->vFaces)
	{
		if (HasVertex(pFace, pVtx2))
		{
			UnionPushBack(pVtx1->vNeighbor, pVtx2);
			UnionPushBack(pVtx2->vNeighbor, pVtx1);
			return;
		}
	}

	EraseFirst(pVtx1->vNeighbor, pVtx2);
	EraseFirst(pVtx2->vNeighbor, pVtx1);
}

void MMeshDetailMap::ComputeNormal(Face* pFace)
{
	Vector3& v0 = pFace->vIndices[0]->pos;
	Vector3& v1 = pFace->vIndices[1]->pos;
	Vector3& v2 = pFace->vIndices[2]->pos;

	pFace->v3Normal = (v1 - v0).CrossProduct(v2 - v1);
	pFace->v3Normal.Normalize();
}

float MMeshDetailMap::GetCollapseCost(Vertex* pFrom, Vertex* pTo)
{
	float edgelength = (pFrom->pos - pTo->pos).Length();
	float curvature = 0;

	std::vector<Face*> vSides;
	for (Face* pFace : pFrom->vFaces)
	{
		if (HasVertex(pFace, pTo))
		{
			vSides.push_back(pFace);
		}
	}

	for (Face* pFromFace : pFrom->vFaces)
	{
		float mincurv = 1;
		for (Face* pSideFace : vSides)
		{
			float dotprod = pFromFace->v3Normal * pSideFace->v3Normal;
			mincurv = MIN(mincurv, (1 - dotprod) / 2.0f);
		}
		curvature = MAX(curvature, mincurv);
	}

	return edgelength * curvature;
}

void MMeshDetailMap::UpdateCollapse(Vertex* pVertex)
{
	if (pVertex->vNeighbor.empty())
	{
		pVertex->pCollapseVertex = nullptr;
		pVertex->fObjdist = -1.0f;
		return;
	}

	pVertex->fObjdist = FLT_MAX;
	pVertex->pCollapseVertex = nullptr;

	for (Vertex* pNeighbor : pVertex->vNeighbor) {
		float fDist = GetCollapseCost(pVertex, pNeighbor);
		if (fDist < pVertex->fObjdist) {
			pVertex->pCollapseVertex = pNeighbor;
			pVertex->fObjdist = fDist;
		}
	}
}

void MMeshDetailMap::Collapse(Vertex* pFrom, Vertex* pTo)
{
	std::vector<Vertex*> tmp = pFrom->vNeighbor;

	for (int i = pFrom->vFaces.size() -1; i >=0; --i)
	{
		Face* pFace = pFrom->vFaces[i];
		if (HasVertex(pFace, pTo))
		{
			Unuse(pFace);
			delete pFace;
		}
	}

	for (int i = pFrom->vFaces.size() - 1; i >= 0; --i)
	{
		Face* pFace = pFrom->vFaces[i];
		ReplaceVertex(pFace, pFrom, pTo);
	}

	for (Vertex* pVertex : tmp)
	{
		UpdateCollapse(pVertex);
	}
}

MMeshDetailMap::Vertex* MMeshDetailMap::GetMinCollapseCostVertex(std::vector<Vertex*>& vVertices)
{
	Vertex* pMinVertex = vVertices[0];
	for (Vertex* pVertex : vVertices)
	{
		if (pMinVertex->fObjdist > pVertex->fObjdist)
			pMinVertex = pVertex;
	}

	return pMinVertex;
}
