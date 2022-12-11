#include "MMergeInstancingTransformParam.h"

MMergeInstancingTransformParam::MMergeInstancingTransformParam()
	: MShaderConstantParam()
{
	unSet = 1;
	unBinding = 0;
	eShaderType = MEShaderParamType::EVertex;

	MStruct worldMatrixSrt;

	MVariantArray matCamProjArray;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::MERGE_INSTANCING_MAX_NUM; ++nCascadedIdx)
	{
		matCamProjArray.AppendValue<MMeshMatrix>();
	}
	worldMatrixSrt.SetValue("U_mat", matCamProjArray);

	var = worldMatrixSrt;
}
