/**
 * @File         MMultiLevelMesh
 * 
 * @Created      2020-03-19 21:32:09
 *
 * @Author       DoubleYe
 *
 * https://github.com/melax/sandbox
**/

#ifndef _M_MMULTILEVELMESH_H_
#define _M_MMULTILEVELMESH_H_
#include "MMesh.h"
#include "MResource.h"
#include <vector>

class MORTY_API MMultiLevelMesh
{
public:
	struct Face;
	struct Vertex
	{
		Vector3 pos;
		std::vector<struct Vertex*> vNeighbor;
		std::vector<struct Face*> vFaces;

		uint32_t unVertexIndex;
		float fObjdist;
		Vertex* pCollapseVertex;
	};

	struct Face
	{
		Vertex* vIndices[3];
		Vector3 v3Normal;
	};

public:
	MMultiLevelMesh();
	virtual ~MMultiLevelMesh() {}

	void BindMesh(const MIMesh* pMesh);
	MIMesh* GetLevel(uint32_t unLevel);

protected:

	MIMesh* CreateLevel(const uint32_t& unIndexNumber);

	void Unuse(Vertex* pVertex);
	void Unuse(Face* pFace);
	void Clean();

	bool HasVertex(Face* pFace, Vertex* pVertex);
	void ReplaceVertex(Face* pFace, Vertex* pFrom, Vertex* pTo);
	void UpdateNeighbor(Vertex* pVtx1, Vertex* pVtx2);

	void ComputeNormal(Face* pFace);
	float GetCollapseCost(Vertex* pFrom, Vertex* pTo);
	void UpdateCollapse(Vertex* pVertex);

	void Collapse(Vertex* u, Vertex* v);

	Vertex* GetMinCollapseCostVertex(std::vector<Vertex*>& vVertices);

public:

	std::vector<uint32_t> m_vIndexToMap;
	std::vector<int> m_vMap;

	MByte* m_pSortVertices;

	const MIMesh* m_pMesh;

	std::vector<MIMesh*> m_vMeshesCache;
};

#endif
