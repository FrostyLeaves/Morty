/**
 * @File         MModelDetailLevel
 * 
 * @Created      2020-03-19 21:32:09
 *
 * @Author       Pobrecito
 *
 * https://github.com/melax/sandbox
**/

#ifndef _M_MODELDETAILLEVEL_H_
#define _M_MODELLEVELDETAIL_H_
#include "MMesh.h"
#include "MResource.h"
#include <vector>

class MModelResource;

class MORTY_CLASS MMeshDetailMap
{
public:
	struct Face;
	struct Vertex
	{
		Vector3 pos;
		std::vector<struct Vertex*> vNeighbor;
		std::vector<struct Face*> vFaces;

		unsigned int unVertexIndex;
		float fObjdist;
		Vertex* pCollapseVertex;
	};

	struct Face
	{
		Vertex* vIndices[3];
		Vector3 v3Normal;
	};

public:
	MMeshDetailMap();
	virtual ~MMeshDetailMap() {}

	void BindMesh(const MIMesh* pMesh);

	MIMesh* CreateLevel(const unsigned int& unIndexNumber);
	MIMesh* GetLevel(unsigned int unLevel);

protected:

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

template<typename T>
void UnionPushBack(std::vector<T>& vector, const T& value)
{
	for (T& v : vector)
		if (v == value) return;
	vector.push_back(value);
}

template<typename T>
void EraseFirst(std::vector<T>& vector, const T& value)
{
	for (std::vector<T>::iterator iter = vector.begin(); iter != vector.end(); ++iter)
	{
		if (*iter == value)
		{
			vector.erase(iter);
			return;
		}
	}
}

	std::vector<unsigned int> m_vIndexToMap;
	std::vector<int> m_vMap;

	MByte* m_pSortVertices;

	const MIMesh* m_pMesh;

	std::vector<MIMesh*> m_vMeshesCache;
};

#endif
