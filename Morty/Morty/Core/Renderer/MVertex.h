/**
 * @File         MVertex
 * 
 * @Created      2019-08-25 15:08:54
 *
 * @Author       Morty
**/

#ifndef _M_MVERTEX_H_
#define _M_MVERTEX_H_
#include "MGlobal.h"
#include "Vector.h"
#include "MString.h"


//Bones number per Vertex.
#define MBONES_PER_VERTEX (4)

//顶点
struct MVertex
{
	Vector3 position;
	Vector3 normal;
	Vector2 texCoords;
	Vector3 tangent;
	Vector3 bitangent;
};

//带骨骼顶点
struct MVertexWithBones
{
	MVertexWithBones()
		: bonesID{0}
		, bonesWeight{0}
	{
	}
	Vector3 position;
	Vector3 normal;
	Vector2 texCoords;
	Vector3 tangent;
	Vector3 bitangent;

	int bonesID[MBONES_PER_VERTEX];
	int bonesWeight[MBONES_PER_VERTEX];
};

#endif