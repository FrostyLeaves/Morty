#ifndef _M_MERGE_INSTANCING_TRANSFORM_PARAM_H_
#define _M_MERGE_INSTANCING_TRANSFORM_PARAM_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Material/MShaderParam.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MRenderableMeshComponent;

struct MMeshCluster
{
	size_t unBeginOffset;
};


class MMergeInstancingTransformParam : public MShaderConstantParam
{
public:
	explicit  MMergeInstancingTransformParam();

public:

	struct MMeshMatrix
	{
		Matrix4 matWorld;
		Matrix3 matNormal;
	};



};


#endif