#include "Model/MMultiLevelMesh.h"

#include "Utility/MLogger.h"
#include "Utility/MFunction.h"
#include "Render/MRenderGlobal.h"

#include <float.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b) )
#define MAX(a, b) ((a) > (b) ? (a) : (b) )

MMultiLevelMesh::MMultiLevelMesh()
	: m_pMesh(nullptr)
	, m_pSortVertices(nullptr)
	, m_vMeshesCache(MRenderGlobal::MESH_LOD_LEVEL_RANGE)
{
	
}

void MMultiLevelMesh::BindMesh(const MIMesh* pMesh)
{
	m_pMesh = pMesh;

	const MByte* pVertices = (MByte*)pMesh->GetVertices();
	const uint32_t* vIndices = pMesh->GetIndices();
	uint32_t unVertexSize = pMesh->GetVertexStructSize();
	uint32_t unVerticesLength = pMesh->GetVerticesNum();
	uint32_t unIndicesLength = pMesh->GetIndicesNum();

	m_vIndexToMap.resize(unVerticesLength);
	m_vMap.resize(unVerticesLength);

	std::vector<Vertex*> vVertices(unVerticesLength);

	for (uint32_t i = 0; i < unVerticesLength; ++i)
	{
		Vertex* pVertex = new Vertex();
		pVertex->unVertexIndex = i;
		pVertex->pos = *((Vector3*)((char*)pVertices + i * unVertexSize));
		pVertex->pCollapseVertex = nullptr;
		vVertices[i] = pVertex;
	}

	for (uint32_t i = 0; i < unIndicesLength; i += 3)
	{
		Face* pFace = new Face();
		pFace->vIndices[0] = vVertices[vIndices[i]];
		pFace->vIndices[1] = vVertices[vIndices[i + 1]];
		pFace->vIndices[2] = vVertices[vIndices[i + 2]];
		ComputeNormal(pFace);

		for (uint32_t n = i; n < i + 3; ++n)
		{
			Vertex* pVtx1 = vVertices[vIndices[n]];
			UNION_PUSH_BACK_VECTOR(pVtx1->vFaces, pFace);
			for (uint32_t m = n + 1; m < i + 3; ++m)
			{
				Vertex* pVtx2 = vVertices[vIndices[m]];
				if (pVtx1 != pVtx2)
				{
					UNION_PUSH_BACK_VECTOR(pVtx1->vNeighbor, pVtx2);
					UNION_PUSH_BACK_VECTOR(pVtx2->vNeighbor, pVtx1);
				}
			}
		}
	}

	for (Vertex* pVertex : vVertices)
		UpdateCollapse(pVertex);

	while (!vVertices.empty())
	{
		Vertex* pVertex = GetMinCollapseCostVertex(vVertices);
		m_vIndexToMap[pVertex->unVertexIndex] = vVertices.size() - 1;
		m_vMap[vVertices.size() - 1] = (pVertex->pCollapseVertex) ? pVertex->pCollapseVertex->unVertexIndex : -1;
		if (pVertex->pCollapseVertex)
			Collapse(pVertex, pVertex->pCollapseVertex);

		ERASE_FIRST_VECTOR(vVertices, pVertex);
		delete pVertex;
	}

	for (uint32_t i = 0; i < m_vMap.size(); i++) {
		m_vMap[i] = (m_vMap[i] == -1) ? 0 : m_vIndexToMap[m_vMap[i]];
	}

	uint32_t unMemorySize = pMesh->GetVerticesSize();
	m_pSortVertices = new MByte[unMemorySize];

	for (uint32_t i = 0; i < unVerticesLength; ++i) {
		memcpy(m_pSortVertices + (m_vIndexToMap[i] * unVertexSize), pVertices + (i * unVertexSize), unVertexSize);
	}
}

MIMesh* MMultiLevelMesh::CreateLevel(const uint32_t& unVertexNumber)
{
	const uint32_t* vIndices = m_pMesh->GetIndices();
	uint32_t unVertexSize = m_pMesh->GetVertexStructSize();
	uint32_t unIndicesLength = m_pMesh->GetIndicesNum();

	MIMesh* pMesh = m_pMesh->Clone();
	uint32_t* vNewIndices = pMesh->GetIndices();
	uint32_t ni = 0;


	for (uint32_t i = 0; i < unIndicesLength; i += 3)
	{
		uint32_t unIndex[3];
		for (uint32_t n = 0; n < 3; ++n)
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

MIMesh* MMultiLevelMesh::GetLevel(uint32_t unLevel)
{
	if (unLevel < 1) unLevel = 1;
	if (unLevel > MRenderGlobal::MESH_LOD_LEVEL_RANGE) unLevel = MRenderGlobal::MESH_LOD_LEVEL_RANGE;

	if (m_vMeshesCache[unLevel] == nullptr)
	{
		uint32_t unVertexNumber = m_pMesh->GetVerticesNum() * (float)unLevel / MRenderGlobal::MESH_LOD_LEVEL_RANGE;
		m_vMeshesCache[unLevel] = CreateLevel(unVertexNumber);
	}

	return m_vMeshesCache[unLevel];
}

void MMultiLevelMesh::Unuse(Vertex* pVertex)
{
	for (Vertex* pNeighbor : pVertex->vNeighbor)
	{
		ERASE_FIRST_VECTOR(pNeighbor->vNeighbor, pVertex);
		ERASE_FIRST_VECTOR(pVertex->vNeighbor, pNeighbor);
	}
}

void MMultiLevelMesh::Unuse(Face* pFace)
{
	for (uint32_t i = 0; i < 3; ++i)
		ERASE_FIRST_VECTOR(pFace->vIndices[i]->vFaces, pFace);
	
	for (uint32_t i = 0; i < 3; ++i)
	{
		for (uint32_t j = i + 1; j < 3; ++j)
		{
			UpdateNeighbor(pFace->vIndices[i], pFace->vIndices[j]);
		}
	}
}

void MMultiLevelMesh::Clean()
{

}

bool MMultiLevelMesh::HasVertex(Face* pFace, Vertex* pVertex)
{
	return pFace->vIndices[0] == pVertex || pFace->vIndices[1] == pVertex || pFace->vIndices[2] == pVertex;
}

void MMultiLevelMesh::ReplaceVertex(Face* pFace, Vertex* pFrom, Vertex* pTo)
{
	//Remove vertex from face
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (pFace->vIndices[i] == pFrom)
			pFace->vIndices[i] = pTo;
	}

	//Remove face from vertex
	ERASE_FIRST_VECTOR(pFrom->vFaces, pFace);

	//Add face to vertex
	UNION_PUSH_BACK_VECTOR(pTo->vFaces, pFace);

	for (uint32_t i = 0; i < 3; ++i)
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

void MMultiLevelMesh::UpdateNeighbor(Vertex* pVtx1, Vertex* pVtx2)
{
	for (Face* pFace : pVtx1->vFaces)
	{
		if (HasVertex(pFace, pVtx2))
		{
			UNION_PUSH_BACK_VECTOR(pVtx1->vNeighbor, pVtx2);
			UNION_PUSH_BACK_VECTOR(pVtx2->vNeighbor, pVtx1);
			return;
		}
	}

	ERASE_FIRST_VECTOR(pVtx1->vNeighbor, pVtx2);
	ERASE_FIRST_VECTOR(pVtx2->vNeighbor, pVtx1);
}

void MMultiLevelMesh::ComputeNormal(Face* pFace)
{
	Vector3& v0 = pFace->vIndices[0]->pos;
	Vector3& v1 = pFace->vIndices[1]->pos;
	Vector3& v2 = pFace->vIndices[2]->pos;

	pFace->v3Normal = (v1 - v0).CrossProduct(v2 - v1);
	pFace->v3Normal.Normalize();
}

float MMultiLevelMesh::GetCollapseCost(Vertex* pFrom, Vertex* pTo)
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
			if (pFromFace != pSideFace)
			{
				float dotprod = pFromFace->v3Normal * pSideFace->v3Normal;
				mincurv = MAX(mincurv, (1 - dotprod) / 2.0f);
			}
		}
		curvature = MAX(curvature, mincurv);
	}

	return edgelength * curvature;
}

void MMultiLevelMesh::UpdateCollapse(Vertex* pVertex)
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

void MMultiLevelMesh::Collapse(Vertex* pFrom, Vertex* pTo)
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

MMultiLevelMesh::Vertex* MMultiLevelMesh::GetMinCollapseCostVertex(std::vector<Vertex*>& vVertices)
{
	Vertex* pMinVertex = vVertices[0];
	for (Vertex* pVertex : vVertices)
	{
		if (pMinVertex->fObjdist > pVertex->fObjdist)
			pMinVertex = pVertex;
	}

	return pMinVertex;
}
