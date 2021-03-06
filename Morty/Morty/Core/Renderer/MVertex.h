﻿/**
 * @File         MVertex
 * 
 * @Created      2019-08-25 15:08:54
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVERTEX_H_
#define _M_MVERTEX_H_
#include "MGlobal.h"
#include "Vector.h"
#include "MString.h"


//顶点
struct MVertex
{
	Vector3 position;

	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texCoords;
};

//带骨骼顶点
struct MVertexWithBones
{
	MVertexWithBones()
	{
		memset(bonesID, 0, sizeof(bonesID));
		memset(bonesWeight, 0, sizeof(bonesID));
	}
	Vector3 position;

	int bonesID[MBONES_PER_VERTEX];
	float bonesWeight[MBONES_PER_VERTEX];

	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texCoords;

};

#endif