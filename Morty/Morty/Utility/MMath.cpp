#include "MMath.h"

Matrix4 MMath::GetScaleAndRotation(const Matrix4& mat)
{
	Matrix4 result = mat;
	result.m[0][3] = 0;
	result.m[1][3] = 0;
	result.m[2][3] = 0;

	return result;
}
