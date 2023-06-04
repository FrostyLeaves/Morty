﻿/**
 * @File         MMeshResourceUtil
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#ifndef _M_MESH_RESOURCE_UTIL_H_
#define _M_MESH_RESOURCE_UTIL_H_
#include "Utility/MGlobal.h"
#include "MMeshResource.h"

struct MMeshResourceData;

class MORTY_API MMeshResourceUtil
{
public:

	static std::unique_ptr<MResourceData> CreatePlane(MEMeshVertexType eVertexType = MEMeshVertexType::Normal, const Vector3& scale = Vector3::One);
	static std::unique_ptr<MResourceData> CreateCube(MEMeshVertexType eVertexType = MEMeshVertexType::Normal);
	static std::unique_ptr<MResourceData> CreateSphere(MEMeshVertexType eVertexType = MEMeshVertexType::Normal);
};

#endif
