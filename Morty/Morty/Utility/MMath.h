/**
 * @File         MMath
 * 
 * @Created      2019-12-10 12:42:08
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMATH_H_
#define _M_MMATH_H_
#include "MGlobal.h"
#include "Matrix.h"

class MORTY_CLASS MMath
{
public:

	template <typename T>
	static T Lerp(const T& a, const T& b, float fSmooth)
	{
		fSmooth = fSmooth < 0 ? 0 : fSmooth > 1.0f ? 1.0f : fSmooth;
		return a * (1 - fSmooth) + b * (fSmooth);
	}


	static Matrix4 GetScaleAndRotation(const Matrix4& mat);

private:

};


#endif
