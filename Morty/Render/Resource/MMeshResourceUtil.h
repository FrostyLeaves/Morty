/**
 * @File         MMeshResourceUtil
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MMeshResource.h"
#include "Mesh/MMeshUtil.h"

struct MMeshResourceData;

class MORTY_API MMeshResourceUtil
{
public:

	static std::unique_ptr<MResourceData> CreatePlane(MEMeshVertexType eVertexType = MEMeshVertexType::Normal);
	static std::unique_ptr<MResourceData> CreateCube(MEMeshVertexType eVertexType = MEMeshVertexType::Normal);
	static std::unique_ptr<MResourceData> CreateSphere(MEMeshVertexType eVertexType = MEMeshVertexType::Normal);
};
